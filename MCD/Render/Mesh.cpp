#include "Pch.h"
#include "Mesh.h"
#include "MeshBuilder.h"
#include "../Core/System/Log.h"

namespace MCD {

Mesh::Mesh(const Path& fileId)
	: Resource(fileId), mImpl(nullptr)
{
	clear();
}

Mesh::~Mesh()
{
	clear();
}

size_t Mesh::bufferSize(size_t bufferIndex) const
{
	MCD_ASSUME(bufferIndex < cMaxBufferCount);
	if(bufferIndex >= bufferCount)
		return 0;

	for(size_t i=0; i<attributeCount; ++i) {
		if(attributes[i].bufferIndex != bufferIndex)
			continue;
		return attributes[i].stride * (bufferIndex == 0 ? indexCount : vertexCount);
	}

	return 0;
}

int Mesh::findAttributeBySemantic(const StringHash& semantic) const
{
	for(size_t i=0; i<attributeCount; ++i) {
		if(attributes[i].format.semantic == semantic)
			return i;
	}
	return -1;
}

const void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped) const
{
	return const_cast<Mesh*>(this)->mapBuffer(bufferIdx, mapped, Mesh::Read);
}

bool Mesh::create(const MeshBuilder& builder, Mesh::StorageHint storageHint)
{
	const size_t attributeCount = builder.attributeCount();
	const size_t bufferCount = builder.bufferCount();

	if(attributeCount > Mesh::cMaxAttributeCount)
		return false;
	if(bufferCount > Mesh::cMaxBufferCount)
		return false;

	MCD_ASSERT(attributeCount > 0 && bufferCount > 0);
	MCD_ASSERT(builder.vertexCount() > 0 && builder.indexCount() > 0);

	this->clear();

	this->bufferCount = bufferCount;
	this->attributeCount = attributeCount;
	this->indexCount = builder.indexCount();
	this->vertexCount = builder.vertexCount();

	for(uint8_t i=0; i<attributeCount; ++i)
	{
		size_t count, stride, bufferId, offset;
		VertexFormat format;

		if(!builder.getAttributePointer(i, &count, &stride, &bufferId, &offset, &format))
			continue;

		Mesh::Attribute& a = this->attributes[i];
		a.format = format;
		a.bufferIndex = uint8_t(bufferId);
		a.byteOffset = uint8_t(offset);
		a.stride = uint16_t(stride);
	}

	const void* data[cMaxBufferCount];
	for(size_t i=0; i<bufferCount; ++i) {
		size_t sizeInByte;
		if(const char* p = builder.getBufferPointer(i, nullptr, &sizeInByte)) {
			MCD_ASSERT(sizeInByte == this->bufferSize(i));
			data[i] = p;
		}
	}
	return create(data, storageHint);
}

Component* MeshComponent2::clone() const
{
	MeshComponent2* cloned = new MeshComponent2;
	cloned->mesh = this->mesh;
	return cloned;
}

}	// namespace MCD
