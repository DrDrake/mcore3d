#include "Pch.h"
#include "Mesh.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

static const size_t cMapArraySize = 65;

// Map the DataType enum into the mHandle array index
static const int cDataType2ComponentCountIndex[cMapArraySize] = {
	-1,				// TextureCoord
	4,				// TextureCoord
	5, 6,			// TextureCoord
	7, 8, 9, 10,	// TextureCoord
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
	mHandle.assign(0);
	mComponentCount.assign(0);
	mComponentCount[cDataType2ComponentCountIndex[Position]] = 3;
	mComponentCount[cDataType2ComponentCountIndex[Index]] = 1;
	mComponentCount[cDataType2ComponentCountIndex[Normal]] = 3;
	mFormat = Position;	// Every mesh at least have the position data
}

Mesh::~Mesh()
{
	// glDeleteBuffers will simple ignore non-valid buffer handles
	if(glDeleteBuffers)
		glDeleteBuffers(mHandle.size(), mHandle.data());
}

void Mesh::bind(DataType dataType)
{
	switch(uint(dataType)) {
	case 1:	case 2:	case 3:	case 4:	case 5:	case 6:	case 7:
		glClientActiveTexture(GL_TEXTURE0 + GLenum (dataType) - 1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
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
		glColorPointer(componentCount(dataType), GL_UNSIGNED_BYTE, 0, nullptr);
		break;

	case Normal:
		glNormalPointer(GL_FLOAT, 0, 0);
		break;

	case Index:
//		glDrawElements(GL_TRIANGLES, nNumIndexes, GL_UNSIGNED_SHORT, 0);
		break;

	case 1:	case 2:	case 3:	case 4:	case 5:	case 6:	case 7:
		{
			uint stride = 0;
			size_t count = mFormat & Mesh::TextureCoord;
			for(size_t i = count; i--;) {
				stride += mComponentCount[cDataType2ComponentCountIndex[dataType]];
			}
			stride *= sizeof(float);

			glBindBuffer(GL_ARRAY_BUFFER, handle(dataType));
			glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
//			glTexCoordPointer(componentCount(dataType), GL_FLOAT, stride, nullptr);
		}
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

	if(mFormat & Mesh::Normal) {
		glEnableClientState(GL_NORMAL_ARRAY);
		bind(Mesh::Normal);
	}

	if(mFormat & Mesh::TextureCoord) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// Bind all available texture coordinates
		const size_t count = mFormat & Mesh::TextureCoord;

		for(size_t i=1; i<=count; ++i)
			bind(Mesh::DataType(i));
	}

	bind(Mesh::Index);

	glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);
}

// Map the DataType enum into the mHandle array index
static const int cDataType2Index[cMapArraySize] = {
	-1,			// Nothing
	4,			// TextureCoord
	4, 4,		// TextureCoord
	4, 4, 4, 4,	// TextureCoord
	0, -1, -1, -1, -1, -1, -1, -1,	// Position
	1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// Index
	2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// Color
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3	// Normal
};

uint Mesh::handle(DataType dataType) const
{
	const uint* handle = const_cast<Mesh*>(this)->getHandlePtr(dataType);

	return handle ? *handle : 0;
}

uint* Mesh::getHandlePtr(DataType dataType)
{
	if(dataType >= cMapArraySize || cDataType2Index[dataType] == -1)
		return nullptr;

	return &mHandle[cDataType2Index[dataType]];
}

uint8_t Mesh::componentCount(DataType dataType) const
{
	if(dataType >= cMapArraySize || cDataType2ComponentCountIndex[dataType] == -1)
		return 0;
	return mComponentCount[cDataType2ComponentCountIndex[dataType]];
}

uint8_t* Mesh::getComponentCountPtr(DataType dataType)
{
	if(dataType >= Mesh::TextureCoord || cDataType2ComponentCountIndex[dataType] == -1)
		return nullptr;
	return &mComponentCount[cDataType2ComponentCountIndex[dataType]];
}

}	// namespace MCD
