#include "Pch.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec4.h"
#include "../Core/System/Log.h"
#include "../Core/System/PtrVector.h"
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

namespace MCD {

class MeshBuilder::Impl
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

int MeshBuilder::declareAttribute(const Semantic& semantic, size_t bufferId)
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

bool MeshBuilder::resizeBuffers(uint16_t vertexCount, size_t indexCount)
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

	Semantic indexSemantic = { "index", sizeof(uint16_t), 1, 0 };
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
	b->elementSize = sizeof(uint16_t);
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
	MCD_ASSERT(mImpl.buffers[0].size() / mImpl.buffers[0].elementSize == mImpl.indexCount);
	return mImpl.indexCount;
}

size_t MeshBuilder::bufferCount() const
{
	return mImpl.buffers.size();
}

int MeshBuilder::findAttributeId(const char* semanticName) const
{
	for(size_t i=0; i<mImpl.attributes.size(); ++i)
		if(::strcmp(mImpl.attributes[i].semantic.name, semanticName) == 0)
			return i;

	return -1;
}

char* MeshBuilder::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, Semantic* semantic)
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

const char* MeshBuilder::getAttributePointer(int attributeId, size_t* count, size_t* stride, size_t* bufferId, Semantic* semantic) const
{
	return const_cast<MeshBuilder*>(this)->getAttributePointer(attributeId, count, stride, bufferId, semantic);
}

char* MeshBuilder::getBufferPointer(size_t bufferIdx,  size_t* elementSize, size_t* sizeInByte)
{
	if(bufferIdx >= mImpl.buffers.size() || mImpl.buffers[bufferIdx].size() == 0)
		return nullptr;

	if(elementSize) *elementSize = mImpl.buffers[bufferIdx].elementSize;
	if(sizeInByte) *sizeInByte = mImpl.buffers[bufferIdx].size();

	return &mImpl.buffers[bufferIdx][0];
}

const char* MeshBuilder::getBufferPointer(size_t bufferIdx,  size_t* elementSize, size_t* sizeInByte) const
{
	return const_cast<MeshBuilder*>(this)->getBufferPointer(bufferIdx, elementSize, sizeInByte);
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

void commitMesh(MeshBuilder& builder, Mesh& mesh, const int* attributeMap, MeshBuilder::StorageHint storageHint)
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

		MeshBuilder::Semantic semantic;
		const char* data = builder.getAttributePointer(attributeId, &count, &stride, nullptr, &semantic);
		const size_t attributeSize = semantic.elementCount * semantic.elementSize;

		if(!data) {
			Log::write(Log::Warn, L"Error occured when committing data from MeshBuilder to Mesh");
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
