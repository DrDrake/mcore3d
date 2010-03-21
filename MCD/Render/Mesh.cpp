#include "Pch.h"
#include "Mesh.h"
#include "MeshBuilder.h"
#include "SemanticMap.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

Mesh::Mesh(const Path& fileId)
	: Resource(fileId)
{
	clear();
}

Mesh::~Mesh()
{
	clear();
}

size_t Mesh::bufferSize(size_t bufferIndex) const
{
	MCD_ASSUME(bufferIndex < cMaxBufferCount);
	if(bufferIndex >= bufferCount)
		return 0;

	for(size_t i=0; i<attributeCount; ++i) {
		if(attributes[i].bufferIndex != bufferIndex)
			continue;
		return attributes[i].stride * (bufferIndex == 0 ? indexCount : vertexCount);
	}

	return 0;
}

static void bindUv(int unit, const Mesh::Attribute& a, const Mesh::Handles& handles)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if(unit != 0)
		glClientActiveTexture(GL_TEXTURE0 + GLenum(unit));

	glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
	glTexCoordPointer(a.elementCount, a.dataType, a.stride, (const void*)a.byteOffset);

	if(unit != 0)
		glClientActiveTexture(GL_TEXTURE0);
}

void Mesh::draw()
{
	if(indexCount == 0)
		return;

	// An array to indicate which attribute is processed by fixed function.
	bool processed[cMaxAttributeCount] = { false };

	MCD_ASSUME(indexAttrIdx > -1);
	MCD_ASSUME(positionAttrIdx > -1);

	processed[indexAttrIdx] = true;
	processed[positionAttrIdx] = true;

	if(normalAttrIdx > -1) {
		processed[normalAttrIdx] = true;
		glEnableClientState(GL_NORMAL_ARRAY);
		const Attribute& a = attributes[normalAttrIdx];
		glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glNormalPointer(a.dataType, a.stride, (const void*)a.byteOffset);
	}

	if(uv0AttrIdx > -1) {
		processed[uv0AttrIdx] = true;
		bindUv(0, attributes[uv0AttrIdx], handles);
	}
	if(uv1AttrIdx > -1) {
		processed[uv1AttrIdx] = true;
		bindUv(1, attributes[uv1AttrIdx], handles);
	}
	if(uv2AttrIdx > -1) {
		processed[uv2AttrIdx] = true;
		bindUv(2, attributes[uv2AttrIdx], handles);
	}

	// Get the current shader program
	GLint shaderProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

	if(shaderProgram > 0) {
		// TODO: Make a [vertex array/shader program] pair cache?
		for(size_t i=1; i<attributeCount; ++i) {
			if(processed[i]) continue;

			const Attribute& a = attributes[i];
			const GLint attributeLocation = glGetAttribLocation(shaderProgram, a.semantic);

			if(attributeLocation == -1)
				Log::format(Log::Warn, "Shader attribute '%s' not found.", a.semantic);

			glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
			glVertexAttribPointer(attributeLocation, a.elementCount, a.dataType, GL_FALSE, a.stride, (const void*)(a.byteOffset));
			glEnableVertexAttribArray(attributeLocation);
		}
		// TODO: Call glDisableVertexAttribArray?
	}

	drawFaceOnly();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Mesh::drawFaceOnly()
{
	MCD_ASSERT(positionAttrIdx > -1 && indexAttrIdx > -1);

	{	// Calling glVertexPointer() as late as possible will have a big performance difference!
		// Reference: http://developer.nvidia.com/object/using_VBOs.html
		glEnableClientState(GL_VERTEX_ARRAY);
		const Attribute& a = attributes[positionAttrIdx];
		glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glVertexPointer(3, a.dataType, a.stride, (const void*)a.byteOffset);
	}

	{	const Attribute& a = attributes[indexAttrIdx];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glDrawElements(GL_TRIANGLES, indexCount, a.dataType, 0);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::clear()
{
	// Invocation of glDeleteBuffers will simple ignore non-valid buffer handles
	if(glDeleteBuffers) {
		// Delete the buffer that is no longer shared.
		for(size_t i=0; i<handles.size(); ++i) {
			if(handles[i] && handles[i].referenceCount() == 1)
				glDeleteBuffers(1, handles[i].get());
			handles[i] = new uint(0);
		}
	}

	attributeCount = 0;
	bufferCount = 0;
	vertexCount = 0;
	indexCount = 0;

	indexAttrIdx = positionAttrIdx = normalAttrIdx =
	uv0AttrIdx = uv1AttrIdx = uv2AttrIdx = -1;
}

MeshPtr Mesh::clone(const char* name, StorageHint hint)
{
	MeshPtr ret = new Mesh(name);

	ret->bufferCount = bufferCount;
	ret->attributes = attributes;
	ret->attributeCount = attributeCount;
	ret->vertexCount = vertexCount;
	ret->indexCount = indexCount;

	ret->indexAttrIdx = indexAttrIdx;
	ret->positionAttrIdx = positionAttrIdx;
	ret->normalAttrIdx = normalAttrIdx;
	ret->uv0AttrIdx = uv0AttrIdx;
	ret->uv1AttrIdx = uv1AttrIdx;
	ret->uv2AttrIdx = uv2AttrIdx;

	MappedBuffers mapped;
	for(size_t i=0; i<bufferCount; ++i) {
		void* data = mapBuffer(i, mapped);
		const GLenum verOrIdxBuf = i == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		uint* handle = ret->handles[i].get();
		MCD_ASSUME(handle);
		glGenBuffers(1, handle);
		glBindBuffer(verOrIdxBuf, *handle);
		glBufferData(verOrIdxBuf, bufferSize(i), data, hint);
	}
	unmapBuffers(mapped);

	return ret;
}

int Mesh::finidAttributeBySemantic(const char* semantic) const
{
	for(size_t i=0; i<attributeCount; ++i) {
		if(::strcmp(attributes[i].semantic, semantic) == 0)
			return i;
	}
	return -1;
}

void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions)
{
	if(bufferIdx >= bufferCount)
		return nullptr;

	if(mapped[bufferIdx])
		return mapped[bufferIdx];

	const size_t indexBufferId = attributes[indexAttrIdx].bufferIndex;
	const GLenum target = (bufferIdx == indexBufferId) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

	GLenum flags = 0;
	if(mapOptions & Read && mapOptions & Write)
		flags = GL_READ_WRITE;
	else if(mapOptions & Read)
		flags = GL_READ_ONLY;
	else if(mapOptions & Write)
		flags = GL_WRITE_ONLY;

	glBindBuffer(target, *handles[bufferIdx]);

	// See the VBO's spec for the optimization about glMapBuffer() using glBufferData() with null data.
	// reference: http://www.songho.ca/opengl/gl_vbo.html
	// reference: http://hacksoflife.blogspot.com/2010/02/double-buffering-vbos.html
	if(mapOptions & Discard)
		glBufferData(target, bufferSize(bufferIdx), nullptr, GL_STATIC_READ);

	void* ret = glMapBuffer(target, flags);
	mapped[bufferIdx] = ret;
	return ret;
}

const void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped) const
{
	return const_cast<Mesh*>(this)->mapBuffer(bufferIdx, mapped, Mesh::Read);
}

void Mesh::unmapBuffers(MappedBuffers& mapped) const
{
	for(size_t i=0; i<bufferCount; ++i) {
		if(!mapped[i])
			continue;

		const size_t indexBufferId = attributes[indexAttrIdx].bufferIndex;
		const GLenum target = (i == indexBufferId) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

		glBindBuffer(target, *handles[i]);
		glUnmapBuffer(target);

		mapped[i] = nullptr;	// Just feeling set it to null is more safe :)
	}
}

static int toGlType(MeshBuilder::ElementType type)
{
	static const int mapping[] = {
		-1,
		GL_INT, GL_UNSIGNED_INT,
		GL_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_UNSIGNED_SHORT,
		GL_FLOAT, GL_DOUBLE,
		-1
	};

	return mapping[type - MeshBuilder::TYPE_NOT_USED];
}

bool commitMesh(const MeshBuilder& builder, Mesh& mesh, Mesh::StorageHint storageHint)
{
	const size_t attributeCount = builder.attributeCount();
	const size_t bufferCount = builder.bufferCount();

	if(attributeCount > Mesh::cMaxAttributeCount)
		return false;
	if(bufferCount > Mesh::cMaxBufferCount)
		return false;

	MCD_ASSERT(attributeCount > 0 && bufferCount > 0);
	MCD_ASSERT(builder.vertexCount() > 0 && builder.indexCount() > 0);

	mesh.clear();

	mesh.bufferCount = bufferCount;
	mesh.attributeCount = attributeCount;
	mesh.indexCount = builder.indexCount();
	mesh.vertexCount = builder.vertexCount();

	for(uint8_t i=0; i<attributeCount; ++i)
	{
		size_t count, stride, bufferId, offset;
		MeshBuilder::Semantic semantic;

		if(!builder.getAttributePointer(i, &count, &stride, &bufferId, &offset, &semantic))
			continue;

		Mesh::Attribute& a = mesh.attributes[i];
		a.dataType = toGlType(semantic.elementType);
		a.elementSize = uint16_t(semantic.elementSize);
		a.elementCount = uint8_t(semantic.elementCount);
		a.bufferIndex = uint8_t(bufferId);
		a.byteOffset = uint8_t(offset);
		a.stride = uint16_t(stride);
		a.semantic = semantic.name;

		// Setup the short cut attribute indices
		const SemanticMap& semanticMap = SemanticMap::getSingleton();
		if(strcmp(semantic.name, semanticMap.index().name) == 0)
			mesh.indexAttrIdx = i;
		else if(strcmp(semantic.name, semanticMap.position().name) == 0)
			mesh.positionAttrIdx = i;
		else if(strcmp(semantic.name, semanticMap.normal().name) == 0)
			mesh.normalAttrIdx = i;
		else if(strcmp(semantic.name, semanticMap.uv(0, a.elementCount).name) == 0)
			mesh.uv0AttrIdx = i;
		else if(strcmp(semantic.name, semanticMap.uv(1, a.elementCount).name) == 0)
			mesh.uv1AttrIdx = i;
		else if(strcmp(semantic.name, semanticMap.uv(2, a.elementCount).name) == 0)
			mesh.uv2AttrIdx = i;
	}

	for(size_t i=0; i<bufferCount; ++i)
	{
		uint* handle = mesh.handles[i].get();
		MCD_ASSUME(handle);
		if(!*handle)
			glGenBuffers(1, handle);

		const GLenum verOrIdxBuf = i == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		glBindBuffer(verOrIdxBuf, *handle);
		size_t sizeInByte;
		if(const char* data = builder.getBufferPointer(i, nullptr, &sizeInByte)) {
			MCD_ASSERT(sizeInByte == mesh.bufferSize(i));
			glBufferData(verOrIdxBuf, sizeInByte, data, storageHint);
		}
	}

	return true;
}

}	// namespace MCD
