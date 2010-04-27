#include "Pch.h"
#include "Skinning.h"
#include "Mesh.h"
#include "../Core/Math/Skeleton.h"

namespace MCD {

void skinning(
	const SkeletonPose& skeleton,
	Mesh& mesh,
	Mesh& basePoseMesh,
	size_t jointIndex,
	size_t weightIndex,
	size_t jointPerVertex,
	int normalIndex)
{
	if(skeleton.jointCount() == 0)
		return;

	Mesh::MappedBuffers mapped, basePoseMapped;
	const Mesh::MapOption writeOption = Mesh::MapOption(Mesh::Write | Mesh::Discard);

	// Ensure the position array is mapped first, to ensure the writeOption is in effect
	StrideArray<Vec3f> position = mesh.mapAttribute<Vec3f>(Mesh::cPositionAttrIdx, mapped, writeOption);

	if(normalIndex == -1) {	// Skinning position only
		skinning(
			position,
			basePoseMesh.mapAttribute<const Vec3f>(Mesh::cPositionAttrIdx, basePoseMapped, Mesh::Read),
			StrideArray<const Mat44f>(&skeleton.transforms[0], skeleton.jointCount()),
			mesh.mapAttributeUnsafe<uint8_t>(jointIndex, mapped),
			mesh.mapAttributeUnsafe<float>(weightIndex, mapped),
			jointPerVertex
		);
	} else {				// Skinning position and normal
		skinning(
			position,
			mesh.mapAttribute<Vec3f>(normalIndex, mapped, writeOption),
			basePoseMesh.mapAttribute<const Vec3f>(Mesh::cPositionAttrIdx, basePoseMapped, Mesh::Read),
			basePoseMesh.mapAttribute<const Vec3f>(normalIndex, basePoseMapped, Mesh::Read),
			StrideArray<const Mat44f>(&skeleton.transforms[0], skeleton.jointCount()),
			mesh.mapAttributeUnsafe<uint8_t>(jointIndex, mapped),
			mesh.mapAttributeUnsafe<float>(weightIndex, mapped),
			jointPerVertex
		);
	}

	mesh.unmapBuffers(mapped);
	basePoseMesh.unmapBuffers(basePoseMapped);
}

}	// namespace MCD
