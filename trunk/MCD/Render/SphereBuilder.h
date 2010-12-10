#ifndef __MCD_RENDER_SPHEREBUILDER__
#define __MCD_RENDER_SPHEREBUILDER__

#include "MeshBuilder.h"

namespace MCD {

//! Create a chamfer box with length = 2
class MCD_RENDER_API SphereBuilder : public MCD::MeshBuilderIM
{
public:
	SphereBuilder(float radius, uint16_t segmentCount);

	//! The index id generated.
	int posId, normalId, uvId;
};	// SphereBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_SPHEREBUILDER__
