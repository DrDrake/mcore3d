#include "Pch.h"

#include "EditableMesh.h"
#include "MeshBuilder.h"	// For ~Mesh() to work

#include "../Core/Math/Vec4.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

class EditableMesh::Impl
{
public:
	Vec3f* mPosPtr;
	Vec3f* mNormPtr;
	
	typedef float* UVPtr;
	UVPtr mUVPtr[Mesh::cMaxTextureCoordCount];

	uint16_t* mIndexPtr;
	size_t mIndexCnt;

	Impl() : mPosPtr(nullptr), mNormPtr(nullptr), mIndexPtr(nullptr)
	{
		for(size_t i=0; i<Mesh::cMaxTextureCoordCount; ++i)
			mUVPtr[i] = nullptr;
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

	size_t cUVCnt = builder->format() & Mesh::TextureCoord;
	MCD_VERIFY(cUVCnt <= Mesh::cMaxTextureCoordCount);
	
	for(size_t i=0; i<cUVCnt; ++i)
		mImpl->mUVPtr[i] = (Impl::UVPtr)builder->acquireBufferPointer(Mesh::TextureCoord0+i);

	mImpl->mIndexPtr	= (uint16_t*)builder->acquireBufferPointer(Mesh::Index, &mImpl->mIndexCnt);
}

void EditableMesh::endEditing(bool commit)
{
#define _RELEASE_PTR(ptr) builder->releaseBufferPointer(ptr); ptr = nullptr;

	_RELEASE_PTR(mImpl->mPosPtr);
	_RELEASE_PTR(mImpl->mNormPtr);

	for(int i=0; i<Mesh::cMaxTextureCoordCount; ++i)
	{
		Impl::UVPtr ptr = mImpl->mUVPtr[i];
		if(ptr) _RELEASE_PTR(ptr);
	}
	
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
	MCD_ASSUME(mImpl && mImpl->mIndexPtr != nullptr);
	return &mImpl->mIndexPtr[face*3];
}

Vec3f& EditableMesh::getPositionAt(uint16_t vertexIndex)
{
	MCD_ASSUME(mImpl && mImpl->mPosPtr != nullptr);
	return mImpl->mPosPtr[vertexIndex];
}

Vec3f& EditableMesh::getNormalAt(uint16_t vertexIndex)
{
	MCD_ASSUME(mImpl && mImpl->mNormPtr != nullptr);
	return mImpl->mNormPtr[vertexIndex];
}

Vec2f& EditableMesh::getUV2dAt(size_t unit, uint16_t vertexIndex)
{
	MCD_ASSUME(unit < Mesh::cMaxTextureCoordCount);
	MCD_ASSUME(mImpl && mImpl->mUVPtr[unit] != nullptr);
	return ((Vec2f*)mImpl->mUVPtr[unit])[vertexIndex];
}

Vec3f& EditableMesh::getUV3dAt(size_t unit, uint16_t vertexIndex)
{
	MCD_ASSUME(unit < Mesh::cMaxTextureCoordCount);
	MCD_ASSUME(mImpl && mImpl->mUVPtr[unit] != nullptr);
	return ((Vec3f*)mImpl->mUVPtr[unit])[vertexIndex];
}

Vec4f& EditableMesh::getUV4dAt(size_t unit, uint16_t vertexIndex)
{
	MCD_ASSUME(unit < Mesh::cMaxTextureCoordCount);
	MCD_ASSUME(mImpl && mImpl->mUVPtr[unit] != nullptr);
	return ((Vec4f*)mImpl->mUVPtr[unit])[vertexIndex];
}

}	// namespace MCD
