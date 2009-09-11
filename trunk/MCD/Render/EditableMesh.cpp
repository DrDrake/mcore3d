#include "Pch.h"

#include "EditableMesh.h"
#include "MeshBuilder.h"	// For ~Mesh() to work

#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

class EditableMesh::Impl
{
public:
	Vec3f* mPosPtr;
	Vec3f* mNormPtr;
	Vec2f* mUV0Ptr;
	Vec2f* mUV1Ptr;
	uint16_t* mIndexPtr;
	size_t mIndexCnt;

	Impl() : mPosPtr(nullptr), mNormPtr(nullptr), mUV0Ptr(nullptr), mUV1Ptr(nullptr), mIndexPtr(nullptr)
	{
	}
};	// Impl

EditableMesh::EditableMesh(const Path& fileId)
	: Mesh(fileId)
	, mImpl(new Impl)
{
}

EditableMesh::EditableMesh(const Path& fileId, const EditableMesh& shareBuffer)
	: Mesh(fileId, shareBuffer)
	, mImpl(new Impl)
{
	this->builder = shareBuffer.builder;
}

EditableMesh::~EditableMesh()
{
}

bool EditableMesh::isEditing() const
{
	return mImpl->mPosPtr != nullptr;
}

void EditableMesh::beginEditing()
{
	MCD_VERIFY(!isEditing());

	mImpl->mPosPtr		= (Vec3f*)builder->acquireBufferPointer(Mesh::Position);
	mImpl->mNormPtr		= (Vec3f*)builder->acquireBufferPointer(Mesh::Normal);
	mImpl->mUV0Ptr		= (Vec2f*)builder->acquireBufferPointer(Mesh::TextureCoord0);
	//mImpl->mUV1Ptr		= (Vec2f*)builder->acquireBufferPointer(Mesh::TextureCoord1);
	mImpl->mIndexPtr	= (uint16_t*)builder->acquireBufferPointer(Mesh::Index, &mImpl->mIndexCnt);
}

void EditableMesh::endEditing(bool commit)
{
#define _RELEASE_PTR(ptr) builder->releaseBufferPointer(ptr); ptr = nullptr;

	_RELEASE_PTR(mImpl->mPosPtr);
	_RELEASE_PTR(mImpl->mNormPtr);
	_RELEASE_PTR(mImpl->mUV0Ptr);
	_RELEASE_PTR(mImpl->mUV1Ptr);
	_RELEASE_PTR(mImpl->mIndexPtr);

#undef _RELEASE_PTR
}

size_t EditableMesh::getTriangleCount() const
{
	//MCD_VERIFY(mImpl->mIndexPtr != nullptr);
	if(nullptr == mImpl->mIndexPtr)
		return 0;

	return mImpl->mIndexCnt / 3;
}

uint16_t* EditableMesh::getTriangleIndexAt(size_t face)
{
	MCD_VERIFY(mImpl->mIndexPtr != nullptr);
	return &mImpl->mIndexPtr[face*3];
}

Vec3f& EditableMesh::getPositionAt(uint16_t vertexIndex)
{
	MCD_VERIFY(mImpl->mPosPtr != nullptr);
	return mImpl->mPosPtr[vertexIndex];
}

Vec3f& EditableMesh::getNormalAt(uint16_t vertexIndex)
{
	MCD_VERIFY(mImpl->mNormPtr != nullptr);
	return mImpl->mNormPtr[vertexIndex];
}

Vec2f& EditableMesh::getUV0At(uint16_t vertexIndex)
{
	MCD_VERIFY(mImpl->mUV0Ptr != nullptr);
	return mImpl->mUV0Ptr[vertexIndex];
}

Vec2f& EditableMesh::getUV1At(uint16_t vertexIndex)
{
	MCD_VERIFY(mImpl->mUV1Ptr != nullptr);
	return mImpl->mUV1Ptr[vertexIndex];
}

}	// namespace MCD
