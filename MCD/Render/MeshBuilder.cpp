#include "Pch.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec4.h"
#include "../Core/System/Log.h"
#include "../Core/System/PtrVector.h"
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

namespace MCD {

class MeshBuilder::Impl
{
public:
	bool assertAttributs() const 
	{
		for(Buffers::const_iterator i=(++buffers.begin()); i!=buffers.end(); ++i)
			if(i->componentSize == 0)
				return false;
		return true;
	}

	struct Attribute {
		size_t offset;		//!< Byte offset of this attribute in the buffer
		size_t bufferId;	//!< Index pointing to the buffer it corresponding to
		VertexFormat format;
	};	// Attribute

	typedef std::vector<Attribute> Attributes;
	Attributes attributes;

	struct Buffer : public std::vector<char> {
		Buffer() { componentSize = 0; }

		//! Size of uint32_t for index buffer, vertex size for vertex buffers.
		size_t componentSize;
	};	// Buffer

	typedef ptr_vector<Buffer> Buffers;
	// Use ptr_vector such that resizing of it will not trigger inner vector's copying.
	Buffers buffers;

	uint16_t vertexCount;
	size_t indexCount;
};	// Impl

MeshBuilder::MeshBuilder(bool isCreatedOnStack)
	: mImpl(*(new Impl))
#ifndef NDEBUG
	, mIsCreatedOnStack(isCreatedOnStack)
#endif
{
	clear();
}

MeshBuilder::~MeshBuilder()
{
	delete &mImpl;
}

int MeshBuilder::declareAttribute(const VertexFormat& format, size_t bufferId)
{
	// bufferId = 0 is reserved for index buffer
	if(format.sizeInByte() == 0 || bufferId == 0)
		return -1;

	if(format.semantic.empty())
		return -1;

	if(findAttributeId(format.semantic.hashValue()) != -1)
		return -1;

	// Create new buffer to match the bufferId param
	while(bufferId >= mImpl.buffers.size())
		mImpl.buffers.push_back(new Impl::Buffer);

	const size_t sizeInBytes = format.sizeInByte();
	Impl::Attribute a = { mImpl.buffers[bufferId].componentSize, bufferId, format };
	mImpl.attributes.push_back(a);

	Impl::Buffer& buf = mImpl.buffers[bufferId];

	{	// Resize the buffer for the new attribute
		size_t oldSize = buf.size();
		size_t newSize = (buf.componentSize + sizeInBytes) * vertexCount();
		buf.resize(newSize);

		// Rearrange the interleaved content (if any)
		if(oldSize != 0) {
			const size_t oldEleSize = buf.componentSize;
			const size_t newEleSize = oldEleSize + sizeInBytes;

			// We loop form end to begin
			for(size_t i=mImpl.vertexCount; i--;) {
				const char* const src = &buf[i*oldEleSize];
				char* const dest = &buf[i*newEleSize];
				::memcpy(dest, src, oldEleSize);
			}
		}
	}

	buf.componentSize += sizeInBytes;

	return mImpl.attributes.size() - 1;
}

bool MeshBuilder::resizeBuffers(uint16_t vertexCount, size_t indexCount)
{
	if(!mImpl.assertAttributs())
		return false;

	mImpl.buffers[0].resize(indexCount * mImpl.buffers[0].componentSize);

	for(Impl::Buffers::iterator i=(++mImpl.buffers.begin()); i!=mImpl.buffers.end(); ++i)
		i->resize(i->componentSize * vertexCount);

	mImpl.vertexCount = vertexCount;
	mImpl.indexCount = indexCount;

	return true;
}

bool MeshBuilder::resizeVertexBuffer(uint16_t vertexCount)
{
	return resizeBuffers(vertexCount, indexCount());
}

bool MeshBuilder::resizeIndexBuffer(size_t indexCount)
{
	return resizeBuffers(vertexCount(), indexCount);
}

void MeshBuilder::clear()
{
	mImpl.attributes.clear();

	VertexFormat indexSemantic = VertexFormat::get("index");
	// Prepare the index as a default attribute
	Impl::Attribute a = { 0, 0, indexSemantic };
	mImpl.attributes.push_back(a);

	clearBuffers();
}

void MeshBuilder::clearBuffers()
{
	mImpl.buffers.clear();

	// Pre-allocate the index buffer
	Impl::Buffer* b = new Impl::Buffer;
	b->componentSize = sizeof(uint16_t);
	mImpl.buffers.push_back(b);

	mImpl.vertexCount = 0;
	mImpl.indexCount = 0;
}

size_t MeshBuilder::attributeCount() const
{
	return mImpl.attributes.size();
}

uint16_t MeshBuilder::vertexCount() const
{
	return mImpl.vertexCount;
}

size_t MeshBuilder::indexCount() const
{
	MCD_ASSERT(mImpl.buffers[0].size() / mImpl.buffers[0].componentSize == mImpl.indexCount);
	return mImpl.indexCount;
}

size_t MeshBuilder::bufferCount() const
{
	return mImpl.buffers.size();
}

int MeshBuilder::findAttributeId(const StringHash& semantic) const
{
	for(size_t i=0; i<mImpl.attributes.size(); ++i)
		if(mImpl.attributes[i].format.semantic == semantic)
			return i;

	return -1;
}

char* MeshBuilder::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, size_t* offset, VertexFormat* format)
{
	if(attributeId < 0 || size_t(attributeId) >= mImpl.attributes.size())
		return nullptr;

	const Impl::Attribute& a = mImpl.attributes[attributeId];
	const size_t bufferIdx = a.bufferId;
	const size_t componentSize = mImpl.buffers[bufferIdx].componentSize;
	const size_t totalSize = mImpl.buffers[bufferIdx].size();

	if(totalSize == 0)
		return nullptr;

	if(count) *count = totalSize / componentSize;
	if(stride) *stride = componentSize;
	if(bufferId) *bufferId = bufferIdx;
	if(format) *format = a.format;
	if(offset) *offset = a.offset;

	return &mImpl.buffers[bufferIdx][a.offset];
}

const char* MeshBuilder::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, size_t* offset, VertexFormat* format) const
{
	return const_cast<MeshBuilder*>(this)->getAttributePointer(attributeId, count, stride, bufferId, offset, format);
}

char* MeshBuilder::getBufferPointer(size_t bufferIdx,  size_t* componentSize, size_t* sizeInByte)
{
	if(bufferIdx >= mImpl.buffers.size() || mImpl.buffers[bufferIdx].size() == 0)
		return nullptr;

	if(componentSize) *componentSize = mImpl.buffers[bufferIdx].componentSize;
	if(sizeInByte) *sizeInByte = mImpl.buffers[bufferIdx].size();

	return &mImpl.buffers[bufferIdx][0];
}

const char* MeshBuilder::getBufferPointer(size_t bufferIdx,  size_t* componentSize, size_t* sizeInByte) const
{
	return const_cast<MeshBuilder*>(this)->getBufferPointer(bufferIdx, componentSize, sizeInByte);
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

class MeshBuilderIM::Impl2 : public std::vector<std::vector<char> > {};

MeshBuilderIM::MeshBuilderIM(bool isCreatedOnStack)
	: MeshBuilder(isCreatedOnStack)
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

	mImpl.buffers[0].reserve(indexCount * mImpl.buffers[0].componentSize);

	for(Impl::Buffers::iterator i=(++mImpl.buffers.begin()); i!=mImpl.buffers.end(); ++i)
		i->reserve(i->componentSize * vertexCount);

	return true;
}

bool MeshBuilderIM::vertexAttribute(int attributeId, const void* data)
{
	const size_t attributeCount = mImpl.attributes.size();
	if(attributeId < 1 || size_t(attributeId) >= attributeCount)
		return false;

	mImpl2.resize(attributeCount);
	const VertexFormat& fmt = mImpl.attributes[attributeId].format;
	const size_t size = fmt.sizeInByte();
	mImpl2[attributeId].assign((char*)data, (char*)data + size);
	return true;
}

uint16_t MeshBuilderIM::addVertex()
{
	const uint16_t oldVeretxCount = vertexCount();

	if(oldVeretxCount == std::numeric_limits<uint16_t>::max()) {
		Log::write(Log::Error, "Maximum number of vertex reached in MeshBuilderIM. Try to split your mesh into multiple parts.");
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
		const size_t stride = mImpl.buffers[a.bufferId].componentSize;
		const size_t attributeSize = a.format.sizeInByte();

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
	MCD_ASSERT(sizeof(tmp) == buffer.componentSize * 3);

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

}	// namespace MCD
