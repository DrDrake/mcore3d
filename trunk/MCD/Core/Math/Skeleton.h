#ifndef __MCD_CORE_MATH_SKELETON__
#define __MCD_CORE_MATH_SKELETON__

#include "Mat44.h"
#include "../ShareLib.h"
#include "../System/Array.h"

namespace MCD {

MCD_CORE_API void skinning(
	const StrideArray<Vec3f>& outPos,
	const StrideArray<const Vec3f>& basePosePos,
	const StrideArray<const Mat44f>& joints,
	const StrideArray<uint8_t>& jointIndice,
	const StrideArray<float>& weight,
	size_t jointPerVertex);

MCD_CORE_API void skinning(
	const StrideArray<Vec3f>& outPos,
	const StrideArray<Vec3f>& outNormal,
	const StrideArray<const Vec3f>& basePosePos,
	const StrideArray<const Vec3f>& basePoseNormal,
	const StrideArray<const Mat44f>& joints,
	const StrideArray<uint8_t>& jointIndice,
	const StrideArray<float>& weight,
	size_t jointPerVertex);

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SKELETON__
