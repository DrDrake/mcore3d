#ifndef __MCD_RENDER_PLANEMESHBUILDER__
#define __MCD_RENDER_PLANEMESHBUILDER__

#include "MeshBuilder.h"

namespace MCD {

//! Create a plane mesh
class MCD_RENDER_API PlaneMeshBuilder : public MCD::MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	PlaneMeshBuilder(float width, float height, size_t widthSegmentCount, size_t heightSegmentCount, bool includeTangents = true);

	void commit(MCD::Mesh& mesh, StorageHint storageHint);
};	// PlaneMeshBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_PLANEMESHBUILDER__
