#include "Pch.h"
#include "../Mesh.h"
#include "../MeshBuilder.h"
#include "../../Core/System/Log.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

static void bindUv(int unit, const Mesh::Attribute& a, const Mesh::Handles& handles)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if(unit != 0)
		glClientActiveTexture(GL_TEXTURE0 + GLenum(unit));

	glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
	glTexCoordPointer(
		a.format.componentCount,
		VertexFormat::toApiDependentType(a.format.componentType),
		a.stride, (const void*)a.byteOffset
	);

	if(unit != 0)
		glClientActiveTexture(GL_TEXTURE0);
}

void Mesh::draw()
{
	if(indexCount == 0)
		return;

	// An array to indicate which attribute is processed by fixed function.
	bool processed[cMaxAttributeCount] = { false };

	processed[cIndexAttrIdx] = true;
	processed[cPositionAttrIdx] = true;

	// Index and position attribute are assumed on index 0 and 1
	for(size_t i=2; i<attributes.size(); ++i) {
		const Attribute& a = attributes[i];
		if(a.format.semantic == StringHash("normal")) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
			glNormalPointer(VertexFormat::toApiDependentType(a.format.componentType), a.stride, (const void*)a.byteOffset);
		}
		else if(a.format.semantic == StringHash("uv0"))
			bindUv(0, a, handles);
		else if(a.format.semantic == StringHash("uv1"))
			bindUv(1, a, handles);
		else if(a.format.semantic == StringHash("uv2"))
			bindUv(2, a, handles);
		else
			continue;
		processed[i] = true;
	}

	// Get the current shader program
	GLint shaderProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

	if(shaderProgram > 0) {
		// TODO: Make a [vertex array/shader program] pair cache?
		for(size_t i=1; i<attributeCount; ++i) {
			if(processed[i]) continue;

			const Attribute& a = attributes[i];
			const GLint attributeLocation = glGetAttribLocation(shaderProgram, a.format.semantic.c_str());

			if(attributeLocation == -1)
				Log::format(Log::Warn, "Shader attribute '%s' not found.", a.format.semantic.c_str());

			glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
			glVertexAttribPointer(
				attributeLocation,
				a.format.componentCount,
				VertexFormat::toApiDependentType(a.format.componentType),
				GL_FALSE,
				a.stride,
				(const void*)(a.byteOffset)
			);
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
	{	// Calling glVertexPointer() as late as possible will have a big performance difference!
		// Reference: http://developer.nvidia.com/object/using_VBOs.html
		glEnableClientState(GL_VERTEX_ARRAY);
		const Attribute& a = attributes[cPositionAttrIdx];
		glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glVertexPointer(3, VertexFormat::toApiDependentType(a.format.componentType), a.stride, (const void*)a.byteOffset);
	}

	{	const Attribute& a = attributes[cIndexAttrIdx];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glDrawElements(GL_TRIANGLES, indexCount, VertexFormat::toApiDependentType(a.format.componentType), 0);
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
}

MeshPtr Mesh::clone(const char* name, StorageHint hint)
{
	MeshPtr ret = new Mesh(name);

	ret->bufferCount = bufferCount;
	ret->attributes = attributes;
	ret->attributeCount = attributeCount;
	ret->vertexCount = vertexCount;
	ret->indexCount = indexCount;

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

void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions)
{
	if(bufferIdx >= bufferCount)
		return nullptr;

	if(mapped[bufferIdx])
		return mapped[bufferIdx];

	const size_t indexBufferId = attributes[cIndexAttrIdx].bufferIndex;
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

void Mesh::unmapBuffers(MappedBuffers& mapped) const
{
	for(size_t i=0; i<bufferCount; ++i) {
		if(!mapped[i])
			continue;

		const size_t indexBufferId = attributes[cIndexAttrIdx].bufferIndex;
		const GLenum target = (i == indexBufferId) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

		glBindBuffer(target, *handles[i]);
		glUnmapBuffer(target);

		mapped[i] = nullptr;	// Just feeling set it to null is more safe :)
	}
}

// TODO: Put it back to API specific localtion
int VertexFormat::toApiDependentType(ComponentType type)
{
	static const int mapping[] = {
		-1,
		GL_INT, GL_UNSIGNED_INT,
		GL_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_UNSIGNED_SHORT,
		GL_FLOAT, GL_DOUBLE,
		-1
	};

	return mapping[type - VertexFormat::TYPE_NOT_USED];
}

bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	for(size_t i=0; i<bufferCount; ++i)
	{
		uint* handle = this->handles[i].get();
		MCD_ASSUME(handle);
		if(!*handle)
			glGenBuffers(1, handle);

		const GLenum verOrIdxBuf = i == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		glBindBuffer(verOrIdxBuf, *handle);
		if(const char* p = reinterpret_cast<const char*>(data[i]))
			glBufferData(verOrIdxBuf, bufferSize(i), p, storageHint);
	}
	return true;
}

bool Mesh::create(const MeshBuilder& builder, Mesh::StorageHint storageHint)
{
	const size_t attributeCount = builder.attributeCount();
	const size_t bufferCount = builder.bufferCount();

	if(attributeCount > Mesh::cMaxAttributeCount)
		return false;
	if(bufferCount > Mesh::cMaxBufferCount)
		return false;

	MCD_ASSERT(attributeCount > 0 && bufferCount > 0);
	MCD_ASSERT(builder.vertexCount() > 0 && builder.indexCount() > 0);

	this->clear();

	this->bufferCount = bufferCount;
	this->attributeCount = attributeCount;
	this->indexCount = builder.indexCount();
	this->vertexCount = builder.vertexCount();

	for(uint8_t i=0; i<attributeCount; ++i)
	{
		size_t count, stride, bufferId, offset;
		VertexFormat format;

		if(!builder.getAttributePointer(i, &count, &stride, &bufferId, &offset, &format))
			continue;

		Mesh::Attribute& a = this->attributes[i];
		a.format = format;
		a.bufferIndex = uint8_t(bufferId);
		a.byteOffset = uint8_t(offset);
		a.stride = uint16_t(stride);
	}

	const void* data[cMaxBufferCount];
	for(size_t i=0; i<bufferCount; ++i) {
		size_t sizeInByte;
		if(const char* p = builder.getBufferPointer(i, nullptr, &sizeInByte)) {
			MCD_ASSERT(sizeInByte == this->bufferSize(i));
			data[i] = p;
		}
	}
	return create(data, storageHint);
}

}	// namespace MCD