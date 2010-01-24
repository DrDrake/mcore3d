#include "Pch.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "Color.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec4.h"
#include "../Core/System/Log.h"
#include "../../3Party/glew/glew.h"
#include <limits>
#include <string.h>	// for strlen
#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <stack>
#ifdef MCD_VC
#	pragma warning(pop)
#endif

#include "../Core/System/PtrVector.h"

namespace MCD {

class MeshBuilder2::Impl
{
public:
	bool assertAttributs() const 
	{
		for(Buffers::const_iterator i=(++buffers.begin()); i!=buffers.end(); ++i)
			if(i->elementSize == 0)
				return false;
		return true;
	}

	struct Attribute {
		size_t offset;		//!< Byte offset of this attribute in the buffer
		size_t bufferId;	//!< Index pointing to the buffer it corresponding to
		Semantic semantic;
	};	// Attribute

	typedef std::vector<Attribute> Attributes;
	Attributes attributes;

	struct Buffer : public std::vector<char> {
		Buffer() { elementSize = 0; }

		//! Size of uint32_t for index buffer, vertex size for vertex buffers.
		size_t elementSize;
	};	// Buffer

	typedef ptr_vector<Buffer> Buffers;
	// Use ptr_vector such that resizing of it will not trigger inner vector's copying.
	Buffers buffers;

	uint16_t vertexCount;
	size_t indexCount;
};	// Impl

MeshBuilder2::MeshBuilder2(bool isCreatedOnStack)
	: mImpl(*(new Impl))
#ifndef NDEBUG
	, mIsCreatedOnStack(isCreatedOnStack)
#endif
{
	clear();
}

MeshBuilder2::~MeshBuilder2()
{
	delete &mImpl;
}

int MeshBuilder2::declareAttribute(const Semantic& semantic, size_t bufferId)
{
	// bufferId = 0 is reserved for index buffer
	if(semantic.elementCount * semantic.elementSize == 0 || bufferId == 0)
		return -1;

	if(semantic.name == nullptr || ::strlen(semantic.name) == 0)
		return -1;

	if(findAttributeId(semantic.name) != -1)
		return -1;

	// Create new buffer to match the bufferId param
	while(bufferId >= mImpl.buffers.size())
		mImpl.buffers.push_back(new Impl::Buffer);

	const size_t sizeInBytes = semantic.elementCount * semantic.elementSize;
	Impl::Attribute a = { mImpl.buffers[bufferId].elementSize, bufferId, semantic };
	mImpl.attributes.push_back(a);

	Impl::Buffer& buf = mImpl.buffers[bufferId];

	{	// Resize the buffer for the new attribute
		size_t oldSize = buf.size();
		size_t newSize = (buf.elementSize + sizeInBytes) * vertexCount();
		buf.resize(newSize);

		// Rearrange the interleaved content (if any)
		if(oldSize != 0) {
			const size_t oldEleSize = buf.elementSize;
			const size_t newEleSize = oldEleSize + sizeInBytes;

			// We loop form end to begin
			for(size_t i=mImpl.vertexCount; i--;) {
				const char* const src = &buf[i*oldEleSize];
				char* const dest = &buf[i*newEleSize];
				::memcpy(dest, src, oldEleSize);
			}
		}
	}

	buf.elementSize += sizeInBytes;

	return mImpl.attributes.size() - 1;
}

bool MeshBuilder2::resizeBuffers(uint16_t vertexCount, size_t indexCount)
{
	if(!mImpl.assertAttributs())
		return false;

	mImpl.buffers[0].resize(indexCount * mImpl.buffers[0].elementSize);

	for(Impl::Buffers::iterator i=(++mImpl.buffers.begin()); i!=mImpl.buffers.end(); ++i)
		i->resize(i->elementSize * vertexCount);

	mImpl.vertexCount = vertexCount;
	mImpl.indexCount = indexCount;

	return true;
}

bool MeshBuilder2::resizeVertexBuffer(uint16_t vertexCount)
{
	return resizeBuffers(vertexCount, indexCount());
}

bool MeshBuilder2::resizeIndexBuffer(size_t indexCount)
{
	return resizeBuffers(vertexCount(), indexCount);
}

void MeshBuilder2::clear()
{
	mImpl.attributes.clear();

	Semantic indexSemantic = { "index", sizeof(uint16_t), 1, 0 };
	// Prepare the index as a default attribute
	Impl::Attribute a = { 0, 0, indexSemantic };
	mImpl.attributes.push_back(a);

	clearBuffers();
}

void MeshBuilder2::clearBuffers()
{
	mImpl.buffers.clear();

	// Pre-allocate the index buffer
	Impl::Buffer* b = new Impl::Buffer;
	b->elementSize = sizeof(uint16_t);
	mImpl.buffers.push_back(b);

	mImpl.vertexCount = 0;
	mImpl.indexCount = 0;
}

size_t MeshBuilder2::attributeCount() const
{
	return mImpl.attributes.size();
}

uint16_t MeshBuilder2::vertexCount() const
{
	return mImpl.vertexCount;
}

size_t MeshBuilder2::indexCount() const
{
	MCD_ASSERT(mImpl.buffers[0].size() / mImpl.buffers[0].elementSize == mImpl.indexCount);
	return mImpl.indexCount;
}

size_t MeshBuilder2::bufferCount() const
{
	return mImpl.buffers.size();
}

int MeshBuilder2::findAttributeId(const char* semanticName) const
{
	for(size_t i=0; i<mImpl.attributes.size(); ++i)
		if(::strcmp(mImpl.attributes[i].semantic.name, semanticName) == 0)
			return i;

	return -1;
}

char* MeshBuilder2::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, Semantic* semantic)
{
	if(attributeId < 0 || size_t(attributeId) >= mImpl.attributes.size())
		return nullptr;

	const Impl::Attribute& a = mImpl.attributes[attributeId];
	const size_t bufferIdx = a.bufferId;
	const size_t elementSize = mImpl.buffers[bufferIdx].elementSize;
	const size_t totalSize = mImpl.buffers[bufferIdx].size();

	if(totalSize == 0)
		return nullptr;

	if(count) *count = totalSize / elementSize;
	if(stride) *stride = elementSize;
	if(bufferId) *bufferId = bufferIdx;
	if(semantic) *semantic = a.semantic;

	return &mImpl.buffers[bufferIdx][a.offset];
}

const char* MeshBuilder2::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, Semantic* semantic) const
{
	return const_cast<MeshBuilder2*>(this)->getAttributePointer(attributeId, count, stride, bufferId, semantic);
}

char* MeshBuilder2::getBufferPointer(size_t bufferIdx,  size_t* elementSize, size_t* sizeInByte)
{
	if(bufferIdx >= mImpl.buffers.size() || mImpl.buffers[bufferIdx].size() == 0)
		return nullptr;

	if(elementSize) *elementSize = mImpl.buffers[bufferIdx].elementSize;
	if(sizeInByte) *sizeInByte = mImpl.buffers[bufferIdx].size();

	return &mImpl.buffers[bufferIdx][0];
}

const char* MeshBuilder2::getBufferPointer(size_t bufferIdx,  size_t* elementSize, size_t* sizeInByte) const
{
	return const_cast<MeshBuilder2*>(this)->getBufferPointer(bufferIdx, elementSize, sizeInByte);
}

#ifndef NDEBUG
void intrusivePtrAddRef(MeshBuilder2* p) {
	++(p->mRefCount);
}

void intrusivePtrRelease(MeshBuilder2* p) {
	if(--(p->mRefCount) == 0) {
		MCD_ASSERT(!p->mIsCreatedOnStack && "If you want to share MeshBuilder2, please flag it as NOT isCreatedOnStack in the constructor");
		p->mIsCreatedOnStack = true;
		delete p;
	}
}
#endif

class MeshBuilderIM::Impl2 : public std::vector<std::vector<char> >
{
public:
};	// Impl2

MeshBuilderIM::MeshBuilderIM(bool isCreatedOnStack)
	: MeshBuilder2(isCreatedOnStack)
	, mImpl2(*(new Impl2))
{
}

MeshBuilderIM::~MeshBuilderIM()
{
	delete &mImpl2;
}

bool MeshBuilderIM::reserveBuffers(uint16_t vertexCount, size_t indexCount)
{
	if(!mImpl.assertAttributs())
		return false;

	mImpl.buffers[0].reserve(indexCount * mImpl.buffers[0].elementSize);

	for(Impl::Buffers::iterator i=(++mImpl.buffers.begin()); i!=mImpl.buffers.end(); ++i)
		i->reserve(i->elementSize * vertexCount);

	return true;
}

bool MeshBuilderIM::vertexAttribute(int attributeId, const void* data)
{
	const size_t attributeCount = mImpl.attributes.size();
	if(attributeId < 1 || size_t(attributeId) >= attributeCount)
		return false;

	mImpl2.resize(attributeCount);
	const Semantic& s = mImpl.attributes[attributeId].semantic;
	const size_t size = s.elementCount * s.elementSize;
	mImpl2[attributeId].assign((char*)data, (char*)data + size);
	return true;
}

uint16_t MeshBuilderIM::addVertex()
{
	const uint16_t oldVeretxCount = vertexCount();

	if(oldVeretxCount == std::numeric_limits<uint16_t>::max()) {
		Log::write(Log::Error, L"Maximum number of vertex reached in MeshBuilderIM. Try to split your mesh into multiple parts.");
		return uint16_t(-1);
	}

	if(!resizeBuffers(oldVeretxCount + 1, indexCount()))
		return uint16_t(-1);

	const size_t attributeCount = mImpl.attributes.size();
	if(mImpl2.size() != attributeCount)
		return uint16_t(-1);

	// Copy the data
	for(size_t i=1; i<attributeCount; ++i)
	{
		const Impl::Attribute& a = mImpl.attributes[i];
		const size_t stride = mImpl.buffers[a.bufferId].elementSize;
		const size_t attributeSize = a.semantic.elementCount * a.semantic.elementSize;

		// Skip those un-assigned attribute
		if(mImpl2[i].size() != attributeSize)
			continue;

		const char* srcPtr = &mImpl2[i][0];
		char* destPtr = &mImpl.buffers[a.bufferId][oldVeretxCount * stride + a.offset];
		::memcpy(destPtr, srcPtr, attributeSize);
	}

	return oldVeretxCount;
}

bool MeshBuilderIM::addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3)
{
	// Check if the 3 indexes are within bound of vertex buffer
	// Only do this checking if the builder is responsible for the vertex buffer,
	// since the MeshBuilder may only use to build index buffer only.
	const uint16_t max = vertexCount();
	if(idx1 >= max || idx2 >= max || idx3 >= max)
		return false;

	if(!mImpl.assertAttributs())
		return false;

	const uint16_t tmp[3] = { idx1, idx2, idx3 };
	Impl::Buffer& buffer = mImpl.buffers[0];
	MCD_ASSERT(sizeof(tmp) == buffer.elementSize * 3);

	buffer.insert(buffer.end(), (char*)(tmp), (char*)(tmp) + sizeof(tmp));
	mImpl.indexCount += 3;

	return true;
}

bool MeshBuilderIM::addQuad(uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t idx4)
{
	if(!addTriangle(idx1, idx2, idx3))
		return false;
	if(!addTriangle(idx3, idx4, idx1))
		return false;
	return true;
}

class MeshBuilder::BufferImpl
{
public:
	Vec3f mPosition;
	ColorRGB8 mColor;
	Vec3f mNormal;

	//! The current active texture unit
	Mesh::DataType mTextureUnit;
	//! Array storing the component size for it's corresponding texture unit. Index 0 is reserved, not used.
	Array<size_t, Mesh::cMaxTextureCoordCount + 1> mTextureCoordSize;
	// The current texture coordinates, the content may interpret as Vec2f, Vec3f, or Vec4f. Index 0 is reserved, not used.
	Array<Array<float,4>, Mesh::cMaxTextureCoordCount + 1> mTextureCoord;

	std::vector<Vec3f> mPositions;
	std::vector<ColorRGB8> mColors;
	std::vector<Vec3f> mNormals;
	std::vector<uint16_t> mIndexes;
	// Index 0 is reserved, not used.
	Array<std::vector<float>, Mesh::cMaxTextureCoordCount + 1> mTextureCoords;

	BufferImpl()
	{
		MCD_ASSERT("There are acquired pointer not released when MeshBuilder is being destroy" && mAcquiredPointers.empty());
	}

	void clear()
	{
		mTextureUnit = Mesh::TextureCoord0;
		mTextureCoordSize.assign(0);

		mPositions.clear();
		mColors.clear();
		mNormals.clear();
		mIndexes.clear();

		for(size_t i=0; i<mTextureCoords.size(); ++i)
			mTextureCoords[i].clear();
	}

	void textureUnit(Mesh::DataType textureUnit)
	{
		mTextureUnit = textureUnit;
	}

	void textureCoordSize(size_t size)
	{
		MCD_ASSERT("Call clear() first before calling textureCoordSize() again" && mTextureCoordSize[mTextureUnit] == 0);
		mTextureCoordSize[mTextureUnit] = size;
		mTextureCoords[mTextureUnit].resize(mPositions.size() * size, 0.0f);
	}

	void assertTextureCoordSize(size_t textureUnit)
	{
		MCD_ASSERT(mTextureCoordSize[textureUnit] == 2 || mTextureCoordSize[textureUnit] == 3 || mTextureCoordSize[textureUnit] == 4);
	}

	void textureCoord(const Vec2f& coord)
	{
		MCD_ASSERT("Invalid texture unit" && size_t(mTextureUnit) < Mesh::cMaxTextureCoordCount);
		MCD_ASSERT(mTextureCoordSize[mTextureUnit] == 2);

		reinterpret_cast<Vec2f&>(mTextureCoord[mTextureUnit]) = coord;
	}

	void textureCoord(const Vec3f& coord)
	{
		MCD_ASSERT("Invalid texture unit" && size_t(mTextureUnit) < Mesh::cMaxTextureCoordCount);
		MCD_ASSERT(mTextureCoordSize[mTextureUnit] == 3);

		reinterpret_cast<Vec3f&>(mTextureCoord[mTextureUnit]) = coord;
	}

	void textureCoord(const Vec4f& coord)
	{
		MCD_ASSERT("Invalid texture unit" && size_t(mTextureUnit) < Mesh::cMaxTextureCoordCount);
		MCD_ASSERT(mTextureCoordSize[mTextureUnit] == 4);

		reinterpret_cast<Vec4f&>(mTextureCoord[mTextureUnit]) = coord;
	}

	void* acquireBufferPointer(Mesh::DataType dataType, size_t* count)
	{
		switch(dataType) {
		case Mesh::Position:
			if(mPositions.empty())
				return nullptr;
			if(count)
				*count = mPositions.size();
			mAcquiredPointers.push(&mPositions[0]);
			break;
		case Mesh::Color:
			if(mColors.empty())
				return nullptr;
			if(count)
				*count = mColors.size();
			mAcquiredPointers.push(&mColors[0]);
			break;
		case Mesh::Normal:
			if(mNormals.empty())
				return nullptr;
			if(count)
				*count = mNormals.size();
			mAcquiredPointers.push(&mNormals[0]);
			break;
		case Mesh::Index:
			if(mIndexes.empty())
				return nullptr;
			if(count)
				*count = mIndexes.size();
			mAcquiredPointers.push(&mIndexes[0]);
			break;
		case Mesh::TextureCoord0:
		case Mesh::TextureCoord1:
		case Mesh::TextureCoord2:
		case Mesh::TextureCoord3:
		case Mesh::TextureCoord4:
		case Mesh::TextureCoord5:
			assertTextureCoordSize(dataType);
			if(mTextureCoords[dataType].empty())
				return nullptr;
			if(count)
				*count = mTextureCoords[dataType].size() / mTextureCoordSize[dataType];
			mAcquiredPointers.push(&mTextureCoords[dataType][0]);
			break;
		default:
			MCD_ASSERT(false);
			return nullptr;
		}
		return mAcquiredPointers.top();
	}

	void releaseBufferPointer(const void* ptr)
	{
		if(mAcquiredPointers.empty())
			return;
		if(ptr != mAcquiredPointers.top())
			return;
		mAcquiredPointers.pop();
	}

	bool noPointerAcquired() const {
		return mAcquiredPointers.empty();
	}

	std::stack<void*> mAcquiredPointers;
};	// BufferImpl

template<>
class Mesh::PrivateAccessor<MeshBuilder>
{
public:
	static uint& handle(Mesh& mesh, Mesh::DataType dataType) {
		const Mesh::HandlePtr& ptr = mesh.handlePtr(dataType);
		MCD_ASSERT(ptr);
		// We should not modify the value of the handle if it's already shared.
		// Note that the variable 'ptr' itself already consume 1 reference count
		MCD_ASSERT(ptr.referenceCount() == 1);
		return *ptr;
	}

	static uint8_t& componentCount(Mesh& mesh, Mesh::DataType dataType) {
		uint8_t* p = mesh.componentCountPtr(dataType);
		MCD_ASSUME(p != nullptr);
		return *p;
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

MeshBuilder::MeshBuilder(bool isCreatedOnStack)
	: mFormat(0)
	, mBuffer(*(new BufferImpl))
#ifndef NDEBUG
	, mIsCreatedOnStack(isCreatedOnStack)
#endif
{
	(void)isCreatedOnStack;
	clear();
}

MeshBuilder::~MeshBuilder()
{
	MCD_ASSERT(mIsCreatedOnStack && "If MeshBuilder is going to be shared, use MeshBuilderPtr");
	delete &mBuffer;
}

void MeshBuilder::enable(uint format)
{
	MCD_ASSERT(format > 0);
	MCD_ASSERT((format & Mesh::TextureCoord) != Mesh::TextureCoord);

	if(format & Mesh::Position) {
		mFormat |= Mesh::Position;
	}

	if(format & Mesh::Color) {
		mFormat |= Mesh::Color;
		ColorRGB8 defualtColor = {255, 255, 255};
		mBuffer.mColors.resize(mBuffer.mPositions.size(), defualtColor);
	}

	if((format & Mesh::Normal) && !(mFormat & Mesh::Normal)) {
		mFormat |= Mesh::Normal;
		mBuffer.mNormals.resize(mBuffer.mPositions.size(), Vec3f::c001);
	}

	if(format & Mesh::Index) {
		mFormat |= Mesh::Index;
	}

	// Loop for all possible texture units
	for(size_t f=Mesh::TextureCoord0; f <= Mesh::cMaxTextureCoordCount; ++f) {
		if((format & f) && !(mFormat & f)) {
			const size_t oldCount = mFormat & Mesh::TextureCoord;
			const size_t newCount = format & Mesh::TextureCoord;
			if(newCount > oldCount)
				mFormat = (mFormat & ~Mesh::TextureCoord) | newCount;
			// Invocation of textureCoordSize() later will resize mBuffer.mTextureCoords,
			// therefore no more thing to do here
		}
	}
}

void MeshBuilder::clear()
{
	mFormat = 0;
	mBuffer.clear();
}

void MeshBuilder::reserveVertex(size_t count)
{
	MCD_ASSERT(mFormat & Mesh::Position);
	mBuffer.mPositions.reserve(count);

	if(mFormat & Mesh::Color)
		mBuffer.mColors.reserve(count);

	if(mFormat & Mesh::Normal)
		mBuffer.mNormals.reserve(count);

	// Loop for all possible texture units
	for(size_t f=Mesh::TextureCoord0; f <= Mesh::cMaxTextureCoordCount; ++f) {
		if(mFormat & f) {
			mBuffer.mTextureCoords[f].reserve(count * mBuffer.mTextureCoordSize[f]);
		}
	}
}

void MeshBuilder::reserveTriangle(size_t count)
{
	MCD_ASSERT(mFormat & Mesh::Index);
	mBuffer.mIndexes.reserve(count * 3);
}

void MeshBuilder::position(const Vec3f& position)
{
	MCD_ASSERT(mFormat & Mesh::Position);
	mBuffer.mPosition = position;
}

void MeshBuilder::color(const ColorRGB8& color)
{
	MCD_ASSERT(mFormat & Mesh::Color);
	mBuffer.mColor = color;
}

void MeshBuilder::normal(const Vec3f& normal)
{
	MCD_ASSERT(mFormat & Mesh::Normal);
	mBuffer.mNormal = normal;
}

void MeshBuilder::textureUnit(int textureUnit)
{
	mBuffer.textureUnit((Mesh::DataType)textureUnit);
}

void MeshBuilder::textureCoordSize(size_t size)
{
	mBuffer.textureCoordSize(size);
}

void MeshBuilder::textureCoord(const Vec2f& coord)
{
	MCD_ASSERT(mFormat & Mesh::TextureCoord);
	mBuffer.textureCoord(coord);
}

void MeshBuilder::textureCoord(const Vec3f& coord)
{
	MCD_ASSERT(mFormat & Mesh::TextureCoord);
	mBuffer.textureCoord(coord);
}

void MeshBuilder::textureCoord(const Vec4f& coord)
{
	MCD_ASSERT(mFormat & Mesh::TextureCoord);
	mBuffer.textureCoord(coord);
}

uint16_t MeshBuilder::addVertex()
{
	MCD_ASSERT(mBuffer.noPointerAcquired());
	MCD_ASSERT(mFormat & Mesh::Position);

	// Check that we wont add vertex more than that we can index
	if(mBuffer.mPositions.size() >= size_t(std::numeric_limits<uint16_t>::max())) {
		Log::write(Log::Error, L"Maximum number of vertex reached in MeshBuilder. Try to split your mesh into multiple parts.");
		return uint16_t(-1);
	}

	mBuffer.mPositions.push_back(mBuffer.mPosition);

	if(mFormat & Mesh::Color)
		mBuffer.mColors.push_back(mBuffer.mColor);

	if(mFormat & Mesh::Normal)
		mBuffer.mNormals.push_back(mBuffer.mNormal);

	// Loop for all enabled texture unit
	const size_t cMaxUnit = (mFormat & Mesh::TextureCoord);
	for(size_t f=Mesh::TextureCoord0; f <= cMaxUnit; ++f) {
		mBuffer.assertTextureCoordSize(f);
		std::vector<float>& vector = mBuffer.mTextureCoords[f];
		float* begin = mBuffer.mTextureCoord[f].data();
		float* end = begin + mBuffer.mTextureCoordSize[f];
		vector.insert(vector.end(), begin, end);
	}

	return uint16_t(mBuffer.mPositions.size() - 1);
}

bool MeshBuilder::addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3)
{
	MCD_ASSERT(mBuffer.noPointerAcquired());
	MCD_ASSERT(mFormat & Mesh::Index);

	// Check if the 3 indexes are within bound of vertex buffer
	// Only do this checking if the builder is responsible for the vertex buffer,
	// since the MeshBuilder may only use to build index buffer only.
	if(mFormat & Mesh::Position) {
		uint16_t max = uint16_t(mBuffer.mPositions.size());
		if(idx1 >= max || idx2 >= max || idx3 >= max)
			return false;
	}

	mBuffer.mIndexes.push_back(idx1);
	mBuffer.mIndexes.push_back(idx2);
	mBuffer.mIndexes.push_back(idx3);
	return true;
}

bool MeshBuilder::addQuad(uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t idx4)
{
	if(!addTriangle(idx1, idx2, idx3))
		return false;
	if(!addTriangle(idx3, idx4, idx1))
		return false;
	return true;
}

void MeshBuilder::commit(Mesh& mesh, StorageHint storageHint)
{
	commit(mesh, mFormat, storageHint);
}

size_t min(size_t v1, size_t v2)
{
	return v1 > v2 ? v2 : v1;
}

void MeshBuilder::commit(Mesh& mesh, uint format, StorageHint storageHint)
{
	MCD_ASSERT(	storageHint == GL_STATIC_DRAW ||
				storageHint == GL_DYNAMIC_DRAW ||
				storageHint == GL_STREAM_DRAW);

	if(!glBufferData)
		return;

	typedef Mesh::PrivateAccessor<MeshBuilder> Accessor;

	Accessor::format(mesh) |= format;

	if(format & mFormat & Mesh::Position && !mBuffer.mPositions.empty()) {
		size_t count = mBuffer.mPositions.size();
		uint* handle = &Accessor::handle(mesh, Mesh::Position);
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(Vec3f),
			&mBuffer.mPositions[0], storageHint);
		Accessor::vertexCount(mesh) = count;
	}

	if(format & mFormat & Mesh::Color && !mBuffer.mColors.empty()) {
		size_t count = mBuffer.mColors.size();
		uint* handle = &Accessor::handle(mesh, Mesh::Color);
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(uint8_t) * 3,	// sizeof(ColorRGB8 may > sizeof(uint8_t) * 3
			&mBuffer.mColors[0], storageHint);
	}

	if(format & mFormat & Mesh::Normal && !mBuffer.mNormals.empty()) {
		size_t count = mBuffer.mNormals.size();
		uint* handle = &Accessor::handle(mesh, Mesh::Normal);
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(Vec3f),
			&mBuffer.mNormals[0], storageHint);
	}

	if(format & mFormat & Mesh::Index && !mBuffer.mIndexes.empty()) {
		size_t count = mBuffer.mIndexes.size();
		uint* handle = &Accessor::handle(mesh, Mesh::Index);
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			count * sizeof(uint16_t),
			&mBuffer.mIndexes[0], storageHint);
		Accessor::indexCount(mesh) = count;
	}

	// Loop for all enabled texture unit
	size_t textureCount = min(format & Mesh::TextureCoord, mBuffer.mTextureCoords.size());
	for(size_t f=Mesh::TextureCoord0; f <= textureCount; ++f) {
		size_t floatCount = mBuffer.mTextureCoords[f].size();
		uint* handle = &Accessor::handle(mesh, Mesh::DataType(f));
		// Set the component count of the corresponding texture unit
		mBuffer.assertTextureCoordSize(f);
		uint8_t componentCount = uint8_t(mBuffer.mTextureCoordSize[f]);
		Accessor::componentCount(mesh, Mesh::DataType(f)) = componentCount;
		if(!*handle)
			glGenBuffers(1, handle);
		glBindBuffer(GL_ARRAY_BUFFER, *handle);
		glBufferData(
			GL_ARRAY_BUFFER,
			floatCount * sizeof(float),
			&(mBuffer.mTextureCoords[f][0]), storageHint);
	}
}

void* MeshBuilder::acquireBufferPointer(int dataType, size_t* count)
{
	return mBuffer.acquireBufferPointer((Mesh::DataType)dataType, count);
}

void MeshBuilder::releaseBufferPointer(const void* ptr)
{
	mBuffer.releaseBufferPointer(ptr);
}

#ifndef NDEBUG
void intrusivePtrAddRef(MeshBuilder* p) {
	++(p->mRefCount);
}

void intrusivePtrRelease(MeshBuilder* p) {
	if(--(p->mRefCount) == 0) {
		MCD_ASSERT(!p->mIsCreatedOnStack && "If you want to share MeshBuilder, please flag it as NOT isCreatedOnStack in the constructor");
		p->mIsCreatedOnStack = true;
		delete p;
	}
}
#endif

void commitMesh(MeshBuilder2& builder, Mesh& mesh, const int* attributeMap, MeshBuilder::StorageHint storageHint)
{
	const size_t attributeCount = builder.attributeCount();

	typedef Mesh::PrivateAccessor<MeshBuilder> Accessor;
	Accessor::vertexCount(mesh) = builder.vertexCount();
	Accessor::indexCount(mesh) = builder.indexCount();
	std::vector<char> conversionBuffer;

	for(size_t i=0; i<attributeCount; ++i)
	{
		size_t count, stride;
		const int attributeId = attributeMap[i * 2];
		if(attributeId < 0)	// Skip those unsupported semantics
			continue;

		MeshBuilder2::Semantic semantic;
		const char* data = builder.getAttributePointer(attributeId, &count, &stride, nullptr, &semantic);
		const size_t attributeSize = semantic.elementCount * semantic.elementSize;

		if(!data) {
			Log::write(Log::Warn, L"Error occured when committing data from MeshBuilder2 to Mesh");
			continue;
		}

		// Convert the interleaved attributes to a single buffer.
		if(stride != attributeSize) {
			conversionBuffer.resize(count * attributeSize);
			const char* src = data;
			char* dest = &conversionBuffer[0];
			for(size_t j=0; j<count; ++j, src += stride, dest += attributeSize)
				::memcpy(dest, src, attributeSize);
			data = &conversionBuffer[0];
		}

		const Mesh::DataType format = Mesh::DataType(attributeMap[i * 2 + 1]);

		// Handle the component count for texture coordinates
		// NOTE: Assuming texture coordinate is only float.
		if(format < Mesh::TextureCoord) {
			Accessor::componentCount(mesh, format) = uint8_t(semantic.elementCount);
			Accessor::format(mesh) = (Accessor::format(mesh) & ~Mesh::TextureCoord);	// Remove any TextureCoord format flag
		}

		Accessor::format(mesh) |= format;

		uint* handle = &Accessor::handle(mesh, format);
		if(!*handle)
			glGenBuffers(1, handle);

		const GLenum verOrIdxBuf = attributeId == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		glBindBuffer(verOrIdxBuf, *handle);
		glBufferData(verOrIdxBuf, count * attributeSize, data, storageHint);
	}
}

}	// namespace MCD
