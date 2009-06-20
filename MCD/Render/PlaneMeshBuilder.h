#ifndef __MCD_RENDERTEST_PLANEMESHBUILDER__
#define __MCD_RENDERTEST_PLANEMESHBUILDER__

#include "../../MCD/Render/MeshBuilder.h"

//! Create a plane mesh
class PlaneMeshBuilder : public MCD::MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	PlaneMeshBuilder(float width, float height, size_t widthSegmentCount, size_t heightSegmentCount, bool includeTangents = true);

	void commit(MCD::Mesh& mesh, StorageHint storageHint);
};	// PlaneMeshBuilder

#endif	// __MCD_RENDERTEST_PLANEMESHBUILDER__
