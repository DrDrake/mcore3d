#include "Pch.h"
#include "MeshWriter.h"
#include "Mesh.h"
#include "../Core/System/Stream.h"
#include <iostream>

namespace MCD {

// TODO: Handle endian problem
bool MeshWriter::write(std::ostream& os, const Mesh& mesh)
{
	if(!os)
		return false;

	// Write the counters first
	MCD::write(os, uint16_t(mesh.bufferCount));
	MCD::write(os, uint16_t(mesh.attributeCount));
	MCD::write(os, uint16_t(mesh.vertexCount));
	MCD::write(os, uint16_t(mesh.indexCount));

	// Write the attribute descriptions
	for(size_t i=0; i<mesh.attributeCount; ++i) {
		// NOTE: For simplicity, Mesh::Attribute::semantic is written but ignored.
		os.write((char*)&mesh.attributes[i], sizeof(Mesh::Attribute));
		MCD::write(os, mesh.attributes[i].semantic);
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
		MCD::write(os, uint32_t(size));
		os.write((char*)data, size);
	}
	mesh.unmapBuffers(mapped);

	return true;
}

}	// namespace MCD
