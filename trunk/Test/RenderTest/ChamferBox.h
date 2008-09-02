#ifndef __MCD_RENDERTEST_CHAMFERBOX__
#define __MCD_RENDERTEST_CHAMFERBOX__

#include "../../MCD/Render/MeshBuilder.h"

//! Create a chamfer box with length = 2
class ChamferBoxBuilder : protected MCD::MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount);

	void commit(MCD::Mesh& mesh, StorageHint storageHint);
};	// ChamferBoxBuilder

#endif	// __MCD_RENDERTEST_CHAMFERBOX__
