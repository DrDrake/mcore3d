#include "Pch.h"
#include "SkinMesh.h"
#include "Skeleton.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Quaternion.h"
#include <map>

namespace MCD {

SkinMesh::SkinMesh() {}

SkinMesh::~SkinMesh() {}

Component* SkinMesh::clone() const
{
	SkinMesh* cloned = new SkinMesh;
	cloned->basePoseMesh = this->basePoseMesh;
	cloned->pose = this->pose;	// This will be re-assigned in postClone()
	return cloned;
}

bool SkinMesh::postClone(const Entity& src, Entity& dest)
{
	return true;
}

static void skinning(
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

static void skinning(
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

			Vec3f tmp = basePosePos[i];
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

static void skinning(
	const std::vector<Mat44f>& joints,
	Mesh& mesh,
	Mesh& basePoseMesh,
	size_t jointIndex,
	size_t weightIndex,
	size_t jointPerVertex,
	int normalIndex)
{
	size_t jointCount = joints.size();
	if(0 == jointCount) return;

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
			StrideArray<const Mat44f>(&joints[0], jointCount),
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
			StrideArray<const Mat44f>(&joints[0], jointCount),
			basePoseMesh.mapAttributeUnsafe<uint8_t>(jointIndex, basePoseMapped, Mesh::Read),
			basePoseMesh.mapAttributeUnsafe<float>(weightIndex, basePoseMapped, Mesh::Read),
			jointPerVertex
		);
	}

	mesh.unmapBuffers(mapped);
	basePoseMesh.unmapBuffers(basePoseMapped);
}

typedef std::map<FixString, IntrusiveWeakPtr<Mesh> > ShadowMesh;
static ShadowMesh gShadowMesh;

static MeshPtr getShadowMesh(Mesh& mesh)
{
	const FixString fileId(mesh.fileId().getString().c_str());
	ShadowMesh::iterator i = gShadowMesh.find(fileId);

	if(i != gShadowMesh.end() && i->second)
		return i->second.getNotNull();

	MeshPtr ret = mesh.clone(fileId, Mesh::Stream);
	gShadowMesh[fileId] = ret.get();
	return ret;
}

static std::vector<Mat44f> gTransforms;

void SkinMesh::draw(void* context, Statistic& statistic)
{
	if(!basePoseMesh) return;

	if(pose && !pose->transforms.empty())
	{
		if(!MeshComponent::mesh || basePoseMesh->fileId() != MeshComponent::mesh->fileId())
			MeshComponent::mesh = getShadowMesh(*basePoseMesh);

		// Performs skinning
		gTransforms = pose->transforms;

		// NOTE: If the inverse was already baked into the animation track, we can skip this multiplication
		for(size_t i=0; i<gTransforms.size(); ++i)
			gTransforms[i] *= pose->skeleton->basePoseInverse[i];

		Mesh& m = *mesh;

		const int blendIndexIdx = m.findAttributeBySemantic("jointIndex");
		const int blendWeightIdx = m.findAttributeBySemantic("jointWeight");

		if(blendIndexIdx != -1 && blendWeightIdx != -1)
			skinning(
				gTransforms, m, *basePoseMesh, blendIndexIdx, blendWeightIdx,
				basePoseMesh->attributes[blendIndexIdx].format.gpuFormat.componentCount,
				m.findAttributeBySemantic(VertexFormat::get("normal").semantic)
			);
	}
	else	// No supplied pose, use the base pose mesh for rendering
		MeshComponent::mesh = basePoseMesh;

	// The actual draw
	MeshComponent::draw(context, statistic);
}

}	// namespace MCD
