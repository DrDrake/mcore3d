#include "Pch.h"
#include "Mesh.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

static const size_t cMapArraySize = 65;

// Map the DataType enum into the mHandles array index
static const int cDataType2Index[cMapArraySize] = {
	-1,				// TextureCoord
	4,				// TextureCoord
	5, 6,			// TextureCoord
	7, 8, 9,		// TextureCoord
	-1,
	0, -1, -1, -1, -1, -1, -1, -1,	// Position
	1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// Index
	2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// Color
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3	// Normal
};

Mesh::Mesh(const Path& fileId)
	:
	Resource(fileId),
	mVertexCount(0), mIndexCount(0)
{
	mHandles.assign(0);
	mComponentCount.assign(0);
	mComponentCount[cDataType2Index[Position]] = 3;
	mComponentCount[cDataType2Index[Color]] = 3;
	mComponentCount[cDataType2Index[Index]] = 1;
	mComponentCount[cDataType2Index[Normal]] = 3;
	mFormat = Position;	// Every mesh at least have the position data
}

Mesh::~Mesh()
{
	// Invocation of glDeleteBuffers will simple ignore non-valid buffer handles
	if(glDeleteBuffers)
		glDeleteBuffers(mHandles.size(), mHandles.data());
}

static void bindTextureCoord(Mesh::DataType textureUnit, int handle, size_t componentCount)
{
	glClientActiveTexture(GL_TEXTURE0 + GLenum(textureUnit) - 1);
	glBindBuffer(GL_ARRAY_BUFFER, handle);
	glTexCoordPointer(componentCount, GL_FLOAT, 0, nullptr);
}

void Mesh::bind(DataType dataType)
{
	switch(uint(dataType)) {
	case Mesh::TextureCoord: case 1: case 2: case 3: case 4: case 5: case 6:
		break;

	case Position:
	case Color:
	case Normal:
		glBindBuffer(GL_ARRAY_BUFFER, handle(dataType));
		break;

	case Index:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle(dataType));
		break;

	default:
		noReturn();
	}

	switch(uint(dataType)) {
	case Position:
		glVertexPointer(3, GL_FLOAT, 0, nullptr);
		break;

	case Color:
		// For vertex color, we only support unsiged byte
		glColorPointer(componentCount(dataType), GL_UNSIGNED_BYTE, 0, nullptr);
		break;

	case Normal:
		glNormalPointer(GL_FLOAT, 0, 0);
		break;

	case Index:
		// Do nothing here, we will call glDrawElements in Mesh::draw()
		break;

	case Mesh::TextureCoord:
		// Bind all available texture coordinates
		for(size_t i=TextureCoord0; i <= (mFormat & Mesh::TextureCoord); ++i) {
			DataType type = DataType(i);
			bindTextureCoord(type, handle(type), componentCount(type));
		}
		break;

	case 1: case 2: case 3: case 4: case 5: case 6:
		bindTextureCoord(dataType, handle(dataType), componentCount(dataType));
		break;

	default:
		noReturn();
	}

	int a = glGetError();
	if(a != GL_NO_ERROR)
		printf("OpenGl has error");
}

void Mesh::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	bind(Mesh::Position);

	if(mFormat & Mesh::Color) {
		glEnableClientState(GL_COLOR_ARRAY);
		bind(Mesh::Color);
	}

	if(mFormat & Mesh::Normal) {
		glEnableClientState(GL_NORMAL_ARRAY);
		bind(Mesh::Normal);
	}

	if(mFormat & Mesh::TextureCoord) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		bind(Mesh::TextureCoord);
	}

	bind(Mesh::Index);

	glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);
}

uint Mesh::handle(DataType dataType) const
{
	const uint* handle = const_cast<Mesh*>(this)->getHandlePtr(dataType);

	return handle ? *handle : 0;
}

uint* Mesh::getHandlePtr(DataType dataType)
{
	if(size_t(dataType) >= cMapArraySize || cDataType2Index[dataType] == -1)
		return nullptr;

	return &mHandles[cDataType2Index[dataType]];
}

uint8_t Mesh::componentCount(DataType dataType) const
{
	if(size_t(dataType) >= cMapArraySize || cDataType2Index[dataType] == -1)
		return 0;
	return mComponentCount[cDataType2Index[dataType]];
}

uint8_t* Mesh::getComponentCountPtr(DataType dataType)
{
	if(dataType >= Mesh::TextureCoord || cDataType2Index[dataType] == -1)
		return nullptr;
	return &mComponentCount[cDataType2Index[dataType]];
}

}	// namespace MCD
