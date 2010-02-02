#ifndef __MCD_RENDER_PLANEMESHBUILDER__
#define __MCD_RENDER_PLANEMESHBUILDER__

#include "MeshBuilder.h"

namespace MCD {

//! Create a plane mesh
class MCD_RENDER_API PlaneMeshBuilder : public MeshBuilderIM
{
public:
	PlaneMeshBuilder(float width, float height, uint16_t widthSegmentCount, uint16_t heightSegmentCount, bool includeTangents = true);
};	// PlaneMeshBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_PLANEMESHBUILDER__
