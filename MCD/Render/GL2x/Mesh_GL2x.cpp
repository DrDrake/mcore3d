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
		a.format.gpuFormat.componentCount,
		a.format.gpuFormat.dataType,
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
			glNormalPointer(a.format.gpuFormat.dataType, a.stride, (const void*)a.byteOffset);
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
				a.format.gpuFormat.componentCount,
				a.format.gpuFormat.dataType,
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
		glVertexPointer(3, a.format.gpuFormat.dataType, a.stride, (const void*)a.byteOffset);
	}

	{	const Attribute& a = attributes[cIndexAttrIdx];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glDrawElements(GL_TRIANGLES, indexCount, a.format.gpuFormat.dataType, 0);
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

bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	for(size_t i=0; i<bufferCount; ++i)
	{
		uint* handle = this->handles[i].get();
		MCD_ASSUME(handle);
		if(!*handle)
			glGenBuffers(1, handle);

		const GLenum verOrIdxBuf = i == Mesh::cIndexAttrIdx ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		glBindBuffer(verOrIdxBuf, *handle);
		if(const char* p = reinterpret_cast<const char*>(data[i]))
			glBufferData(verOrIdxBuf, bufferSize(i), p, storageHint);
	}
	return true;
}

}	// namespace MCD

#include "Renderer.inc"
#include "../Material.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

void MeshComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);

	Entity* e = entity();
	MCD_ASSUME(e);

	if(IMaterialComponent* m = renderer.mCurrentMaterial) {
		RenderItem r = { e, this, m, e->worldTransform() };

		Vec3f pos = r.worldTransform.translation();
		renderer.mViewMatrix.transformPoint(pos);
		const float dist = pos.z;

		if(!m->isTransparent())
			renderer.mOpaqueQueue.insert(*new RenderItemNode(-dist, r));
		else
			renderer.mTransparentQueue.insert(*new RenderItemNode(dist, r));
	}
}

}	// namespace MCD
