#include "Pch.h"
#include "MeshBuilder.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"
#include <limits>

namespace MCD {

class MeshBuilder::BufferImpl
{
public:
	Vec3f mPosition;
	Vec3f mNormal;

	std::vector<Vec3f> mPositions;
	std::vector<Vec3f> mNormals;
	std::vector<uint16_t> mIndexes;

	bool mIsBetweenBeginEnd;
};	// BufferImpl

template<>
class Mesh::PrivateAccessor<MeshBuilder>
{
public:
	static Array<uint,5>& handle(Mesh& mesh) {
		return mesh.mHandle;
	}

	static uint& format(Mesh& mesh) {
		return mesh.mFormat;
	}

	static size_t& vertexCount(Mesh& mesh) {
		return mesh.mVertexCount;
	}

	static size_t& indexCount(Mesh& mesh) {
		return mesh.mIndexCount;
	}
};	// PrivateAccessor

MeshBuilder::MeshBuilder()
	:
	mFormat(0),
	mBuffer(*(new BufferImpl))
{
	mBuffer.mIsBetweenBeginEnd = false;
}

MeshBuilder::~MeshBuilder()
{
	delete &mBuffer;
}

void MeshBuilder::begin(uint format)
{
	MCD_ASSERT(format < Mesh::MaxTypeEntry);
	mBuffer.mIsBetweenBeginEnd = true;
	mFormat = format;
}

void MeshBuilder::end()
{
	mBuffer.mIsBetweenBeginEnd = false;
}

void MeshBuilder::clear()
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	mFormat = 0;
	mBuffer.mPositions.clear();
	mBuffer.mNormals.clear();
	mBuffer.mIndexes.clear();
}

void MeshBuilder::reserveVertex(size_t count)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Position);
	mBuffer.mPositions.reserve(count);

	if(mFormat & Mesh::Normal)
		mBuffer.mNormals.reserve(count);
}

void MeshBuilder::reserveTriangle(size_t count)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Index);
	mBuffer.mIndexes.reserve(count * 3);
}

void MeshBuilder::position(const Vec3f& position)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Position);
	mBuffer.mPosition = position;
}

void MeshBuilder::normal(const Vec3f& normal)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Normal);
	mBuffer.mNormal = normal;
}

void MeshBuilder::textureCoord(Mesh::DataType textureUnit, const Vec3f& coord)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
}

uint16_t MeshBuilder::addVertex()
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Position);

	// Check that we wont add vertex more than that we can index
	if(mBuffer.mPositions.size() >= size_t(std::numeric_limits<uint16_t>::max())) {
		Log::write(Log::Error, L"Maximum number of vertex reached in MeshBuilder. Try to split your mesh into multiple parts.");
		return uint16_t(-1);
	}

	mBuffer.mPositions.push_back(mBuffer.mPosition);

	if(mFormat & Mesh::Normal)
		mBuffer.mNormals.push_back(mBuffer.mNormal);

	return uint16_t(mBuffer.mPositions.size() - 1);
}

bool MeshBuilder::addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3)
{
	MCD_ASSERT(mBuffer.mIsBetweenBeginEnd);
	MCD_ASSERT(mFormat & Mesh::Index);

	uint16_t max = uint16_t(mBuffer.mPositions.size());
	if(idx1 >= max || idx2 >= max || idx3 >= max)
		return false;

	mBuffer.mIndexes.push_back(idx1);
	mBuffer.mIndexes.push_back(idx2);
	mBuffer.mIndexes.push_back(idx3);
	return true;
}

void MeshBuilder::commit(Mesh& mesh, StorageHint storageHint)
{
	MCD_ASSERT(!mBuffer.mIsBetweenBeginEnd);

	MCD_ASSERT(	storageHint == GL_STATIC_DRAW ||
				storageHint == GL_DYNAMIC_DRAW ||
				storageHint == GL_STREAM_DRAW);

	if(!glBufferData || mBuffer.mPositions.size() == 0)
		return;

	typedef Mesh::PrivateAccessor<MeshBuilder> Accessor;

	Accessor::format(mesh) = mFormat;

	MCD_ASSUME(mFormat & Mesh::Position);
	if(mFormat & Mesh::Position) {
		size_t vertexCount = mBuffer.mPositions.size();
		uint* handle = &Accessor::handle(mesh)[0];
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexCount * sizeof(GLfloat) * 3,
			&mBuffer.mPositions[0], storageHint);
		Accessor::vertexCount(mesh) = vertexCount;
	}

	if(mFormat & Mesh::Normal) {
		size_t normalCount = mBuffer.mNormals.size();
		uint* handle = &Accessor::handle(mesh)[3];
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			normalCount * sizeof(GLfloat) * 3,
			&mBuffer.mNormals[0], storageHint);
	}

	if(mFormat & Mesh::Index) {
		size_t indexCount = mBuffer.mIndexes.size();
		uint* handle = &Accessor::handle(mesh)[1];
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indexCount * sizeof(GL_UNSIGNED_SHORT),
			&mBuffer.mIndexes[0], storageHint);
		Accessor::indexCount(mesh) = indexCount;
	}
}

}	// namespace MCD
