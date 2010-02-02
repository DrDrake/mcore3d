#include "Pch.h"
#include "Mesh.h"
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

static void bindUv(int unit, const Mesh::Attribute& a, const Mesh::Handles& handles)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0 + GLenum(unit));
	glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
	glTexCoordPointer(a.elementCount, a.dataType, a.stride, (void*)a.byteOffset);
}

void Mesh::draw()
{
	// Get the current shader program
/*	GLint shaderProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

	if(shaderProgram <= 0)
		return;

	// TODO: Make a [vertex array/shader program] pair cache?

	// Bind vertex buffers
	for(size_t i=1; i<attributeCount; ++i)
	{
		const Attribute& att = attributes[i];
		const GLint attributeLocation = glGetAttribLocation(shaderProgram, att.semantic);
		glBindBuffer(GL_ARRAY_BUFFER, *handles[att.bufferIndex]);
		glVertexAttribPointer(attributeLocation, att.elementCount, att.dataType, GL_FALSE, att.stride, (const void*)(att.byteOffset));
		glEnableVertexAttribArray(attributeLocation);
	}

	// Bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handles[0]);

	// Actual draw
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

	// Unbind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// TODO: Call glDisableVertexAttribArray*/

	if(normalAttrIdx > -1) {
		glEnableClientState(GL_NORMAL_ARRAY);
		const Attribute& a = attributes[normalAttrIdx];
		glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glNormalPointer(a.dataType, a.stride, (void*)a.byteOffset);
	}

	if(uv0AttrIdx > -1)
		bindUv(0, attributes[uv0AttrIdx], handles);
	if(uv1AttrIdx > -1)
		bindUv(1, attributes[uv1AttrIdx], handles);
	if(uv2AttrIdx > -1)
		bindUv(2, attributes[uv2AttrIdx], handles);

	drawFaceOnly();
}

void Mesh::drawFaceOnly()
{
	MCD_ASSERT(positionAttrIdx > -1 && indexAttrIdx > -1);

	{	// Calling glVertexPointer() as late as possible will have a big performance difference!
		// Reference: http://developer.nvidia.com/object/using_VBOs.html
		glEnableClientState(GL_VERTEX_ARRAY);
		const Attribute& a = attributes[positionAttrIdx];
		glBindBuffer(GL_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glVertexPointer(3, a.dataType, a.stride, (void*)a.byteOffset);
	}

	{	const Attribute& a = attributes[indexAttrIdx];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handles[a.bufferIndex]);
		glDrawElements(GL_TRIANGLES, indexCount, a.dataType, 0);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
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

void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions)
{
	if(bufferIdx >= bufferCount)
		return nullptr;

	if(mapped[bufferIdx])
		return mapped[bufferIdx];

	const size_t indexBufferId = attributes[indexAttrIdx].bufferIndex;
	const GLenum target = (bufferIdx == indexBufferId) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

	GLenum flags = 0;
	if(int(mapOptions) & Read && int(mapOptions) & Write)
		flags = GL_READ_WRITE;
	else if(mapOptions & Read)
		flags = GL_READ_ONLY;
	else if(mapOptions & Write)
		flags = GL_WRITE_ONLY;

	glBindBuffer(target, *handles[bufferIdx]);

	return glMapBuffer(target, flags);
}

void Mesh::unmapBuffers(MappedBuffers& mapped)
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

}	// namespace MCD
