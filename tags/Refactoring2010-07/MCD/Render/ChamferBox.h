#ifndef __MCD_RENDER_CHAMFERBOX__
#define __MCD_RENDER_CHAMFERBOX__

#include "MeshBuilder.h"

namespace MCD {

//! Create a chamfer box with length = 2
class MCD_RENDER_API ChamferBoxBuilder : public MCD::MeshBuilderIM
{
public:
	ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount, bool includeTangents = false);

	//! The index id generated.
	int posId, normalId, uvId, tangentId;
};	// ChamferBoxBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_CHAMFERBOX__
