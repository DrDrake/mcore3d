#ifndef __MCD_RENDER_CHAMFERBOX__
#define __MCD_RENDER_CHAMFERBOX__

#include "MeshBuilder.h"

namespace MCD {

//! Create a chamfer box with length = 2
class MCD_RENDER_API ChamferBoxBuilder : public MCD::MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount, bool includeTangents = true);

	void commit(Mesh& mesh, StorageHint storageHint);
};	// ChamferBoxBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_CHAMFERBOX__
