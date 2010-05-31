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

	// Copy all non-skinned vertex data
	for(size_t i=0; i<basePoseMesh.attributeCount; ++i) {
		if(i == Mesh::cIndexAttrIdx || i == Mesh::cPositionAttrIdx || i == jointIndex || i == weightIndex || int(i) == normalIndex)
			continue;

		StrideArray<uint8_t> src = basePoseMesh.mapAttributeUnsafe<uint8_t>(i, basePoseMapped, Mesh::Read);
		StrideArray<uint8_t> dest = mesh.mapAttributeUnsafe<uint8_t>(i, mapped, writeOption);
		const size_t srcSize = basePoseMesh.attributes[i].format.sizeInByte();
		const size_t destSize = mesh.attributes[i].format.sizeInByte();

		MCD_ASSERT(srcSize == destSize);

		if(srcSize == destSize) for(size_t j=0; j<src.size; ++j)
			::memcpy(&dest[j], &src[j], basePoseMesh.attributes[i].format.sizeInByte());
	}

	if(normalIndex == -1) {	// Skinning position only
		skinning(
			position,
			basePoseMesh.mapAttribute<const Vec3f>(Mesh::cPositionAttrIdx, basePoseMapped, Mesh::Read),
			StrideArray<const Mat44f>(&skeleton.transforms[0], skeleton.jointCount()),
			basePoseMesh.mapAttributeUnsafe<uint8_t>(jointIndex, basePoseMapped, Mesh::Read),
			basePoseMesh.mapAttributeUnsafe<float>(weightIndex, basePoseMapped, Mesh::Read),
			jointPerVertex
		);
	} else {				// Skinning position and normal
		skinning(
			position,
			mesh.mapAttribute<Vec3f>(normalIndex, mapped, writeOption),
			basePoseMesh.mapAttribute<const Vec3f>(Mesh::cPositionAttrIdx, basePoseMapped, Mesh::Read),
			basePoseMesh.mapAttribute<const Vec3f>(normalIndex, basePoseMapped, Mesh::Read),
			StrideArray<const Mat44f>(&skeleton.transforms[0], skeleton.jointCount()),
			basePoseMesh.mapAttributeUnsafe<uint8_t>(jointIndex, basePoseMapped, Mesh::Read),
			basePoseMesh.mapAttributeUnsafe<float>(weightIndex, basePoseMapped, Mesh::Read),
			jointPerVertex
		);
	}

	mesh.unmapBuffers(mapped);
	basePoseMesh.unmapBuffers(basePoseMapped);
}

}	// namespace MCD
