#ifndef __MCD_RENDER_SKINNING__
#define __MCD_RENDER_SKINNING__

#include "ShareLib.h"

namespace MCD {

class Mesh;
class SkeletonPose;

MCD_RENDER_API void skinningPositionOnly(const SkeletonPose& skeleton, Mesh& mesh, Mesh& basePoseMesh, size_t jointIndex, size_t weightIndex);

}	// namespace MCD

#endif	// __MCD_RENDER_SKINNING__
