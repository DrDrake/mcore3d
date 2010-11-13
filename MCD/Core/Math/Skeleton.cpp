#include "Pch.h"
#include "Skeleton.h"
#include "Quaternion.h"

namespace MCD {

void skinning(
	const StrideArray<Vec3f>& outPos,
	const StrideArray<const Vec3f>& basePosePos,
	const StrideArray<const Mat44f>& joints,
	const StrideArray<uint8_t>& jointIndice,
	const StrideArray<float>& weight,
	size_t jointPerVertex)
{
	MCD_ASSERT(outPos.size == basePosePos.size);
	MCD_ASSERT(jointIndice.size == weight.size);
	MCD_ASSERT(outPos.size == weight.size);

	for(size_t i=0; i<outPos.size; ++i) {
		Vec3f p(0);
		for(size_t j=0; j<jointPerVertex; ++j) {
			float w = (&weight[i])[j];
			if(w <= 0)	// NOTE: We assume a decending joint weight ordering
				break;
			size_t jointIdx = (&jointIndice[i])[j];

			Vec3f tmp = basePosePos[i];
			joints[jointIdx].transformPoint(tmp);
			p += tmp * w;
		}
		outPos[i] = p;
	}
}

void skinning(
	const StrideArray<Vec3f>& outPos,
	const StrideArray<Vec3f>& outNormal,
	const StrideArray<const Vec3f>& basePosePos,
	const StrideArray<const Vec3f>& basePoseNormal,
	const StrideArray<const Mat44f>& joints,
	const StrideArray<uint8_t>& jointIndice,
	const StrideArray<float>& weight,
	size_t jointPerVertex)
{
	MCD_ASSERT(outPos.size == outNormal.size);
	MCD_ASSERT(basePosePos.size == basePoseNormal.size);
	MCD_ASSERT(outPos.size == basePosePos.size);
	MCD_ASSERT(jointIndice.size == weight.size);
	MCD_ASSERT(outPos.size == weight.size);

	for(size_t i=0; i<outPos.size; ++i) {
		Vec3f p(0), n(0);
		for(size_t j=0; j<jointPerVertex; ++j) {
			float w = (&weight[i])[j];
			if(w <= 0)	// NOTE: We assume a decending joint weight ordering
				break;
			size_t jointIdx = (&jointIndice[i])[j];

			Vec3f tmp = basePosePos[i];	// TODO: FIXME: In Studio sometimes the basePosePos is null
			joints[jointIdx].transformPoint(tmp);
			p += tmp * w;

			tmp = basePoseNormal[i];
			joints[jointIdx].transformNormal(tmp);
			n += tmp * w;
		}
		outPos[i] = p;
		outNormal[i] = n;
	}
}

}	// namespace MCD
