#include "Pch.h"
#include "../Mesh.h"

namespace MCD {

void Mesh::draw() {}

void Mesh::drawFaceOnly() {}

void Mesh::clear() {}

void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions)
{
	return nullptr;
}

void Mesh::unmapBuffers(MappedBuffers& mapped) const {}

bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	return false;
}

void MeshComponent2::render2(void* context) {}

int VertexFormat::toApiDependentType(ComponentType type, size_t)
{
	return 0;
}

}	// namespace MCD
