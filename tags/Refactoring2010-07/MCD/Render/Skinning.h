#ifndef __MCD_RENDER_SKINNING__
#define __MCD_RENDER_SKINNING__

#include "ShareLib.h"

namespace MCD {

class Mesh;
class SkeletonPose;

MCD_RENDER_API void skinning(
	const SkeletonPose& skeleton,
	Mesh& mesh,
	Mesh& basePoseMesh,
	size_t jointIndex,
	size_t weightIndex,
	size_t jointPerVertex,
	int normalIndex	// Pass -1 if no need to skin the normal (assumming both mesh has the same normal attribute index)
);

}	// namespace MCD

#endif	// __MCD_RENDER_SKINNING__
