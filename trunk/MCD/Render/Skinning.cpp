#include "Pch.h"
#include "Skinning.h"
#include "Mesh.h"
#include "../Core/Math/Skeleton.h"

namespace MCD {

void skinningPositionOnly(const SkeletonPose& skeleton, Mesh& mesh, Mesh& basePoseMesh, size_t jointIndex, size_t weightIndex)
{
	if(skeleton.jointCount() == 0)
		return;

	Mesh::MappedBuffers mapped, basePoseMapped;

	skinningPositionOnly(
		mesh.mapAttribute<Vec3f>(mesh.positionAttrIdx, mapped, Mesh::Write),
		basePoseMesh.mapAttribute<const Vec3f>(basePoseMesh.positionAttrIdx, basePoseMapped, Mesh::Read),
		StrideArray<const Mat44f>(&skeleton.transforms[0], skeleton.jointCount()),
		mesh.mapAttribute<const Vec4<uint8_t> >(jointIndex, mapped),
		mesh.mapAttribute<const Vec4f>(weightIndex, mapped)
	);

	mesh.unmapBuffers(mapped);
	basePoseMesh.unmapBuffers(basePoseMapped);
}

}	// namespace MCD
