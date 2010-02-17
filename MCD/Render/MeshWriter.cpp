#include "Pch.h"
#include "MeshWriter.h"
#include "Mesh.h"
#include <iostream>

namespace MCD {

// TODO: Handle endian problem
bool MeshWriter::write(std::ostream& os, const Mesh& mesh)
{
	if(!os)
		return false;

	// Write the counters first
	os << uint16_t(mesh.bufferCount);
	os << uint16_t(mesh.attributeCount);
	os << uint16_t(mesh.vertexCount);
	os << uint16_t(mesh.indexCount);

	// Write the attribute descriptions
	for(size_t i=0; i<mesh.attributeCount; ++i) {
		// NOTE: For simplicity, Mesh::Attribute::semantic is written but ignored.
		os.write((char*)&mesh.attributes[i], sizeof(Mesh::Attribute));
		os << mesh.attributes[i].semantic;
	}

	// Write the buffers
	Mesh::MappedBuffers mapped;
	for(size_t i=0; i<mesh.bufferCount; ++i) {
		const void* data = mesh.mapBuffer(i, mapped);
		const size_t size = mesh.bufferSize(i);
		if(!data || !size) {
			mesh.unmapBuffers(mapped);
			return false;
		}
		os << uint32_t(size);
		os.write((char*)data, size);
	}
	mesh.unmapBuffers(mapped);

	return true;
}

}	// namespace MCD
