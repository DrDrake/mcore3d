#ifndef __MCD_RENDERTEST_CHAMFERBOX__
#define __MCD_RENDERTEST_CHAMFERBOX__

#include "../../MCD/Render/MeshBuilder.h"

//! Create a chamfer box with length = 2
class ChamferBoxBuilder : public MCD::MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount, bool includeTangents = true);

	void commit(MCD::Mesh& mesh, StorageHint storageHint);
};	// ChamferBoxBuilder

#endif	// __MCD_RENDERTEST_CHAMFERBOX__
