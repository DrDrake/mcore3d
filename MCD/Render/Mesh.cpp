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

}	// namespace MCD
