#include "Pch.h"
#include "Skeleton.h"
#include "Quaternion.h"

namespace MCD {

void SkeletonPose::init(size_t jointCount)
{
	transforms.resize(jointCount, Mat44f::cIdentity);
}

Skeleton::Skeleton(const Path& fileId)
	: Resource(fileId)
{}

void Skeleton::init(size_t jointCount)
{
	parents.resize(jointCount);
	names.resize(jointCount);
}

int Skeleton::findJointByName(const wchar_t* name) const
{
	for(size_t i=0; i<names.size(); ++i)
		if(names[i] == name )
			return int(i);
	return -1;
}

Skeleton::~Skeleton()
{}

SkeletonAnimation::SkeletonAnimation(const Path& fileId)
	: Resource(fileId)
{}

SkeletonAnimation::~SkeletonAnimation()
{}

void SkeletonAnimation::applyTo(SkeletonPose& pose, int firstJoint, int lastJoint)
{
	if(!skeleton)
		return;

	if(firstJoint < 0) firstJoint = 0;
	if(lastJoint < 0) lastJoint = pose.transforms.size() - 1;

	MCD_ASSERT(firstJoint < lastJoint);
	MCD_ASSERT(lastJoint < int(pose.transforms.size()));
	MCD_ASSERT(pose.jointCount() == skeleton->parents.size());

	size_t trackPerJoint = anim.subtrackCount() / pose.transforms.size();
	if(trackPerJoint < 0)
		return;

	if(trackPerJoint == 2)	// Translation and rotation
	{
		Mat44f m = Mat44f::cIdentity;
		Mat33f tmp;

		// NOTE: Accessing interpolatedResult will cause race condition with animation thread,
		// but it's fine for smoothing varing data.
		const AnimationInstance::KeyFrames& result = anim.weightedResult;

		MCD_ASSERT(result.data && "Call AnimationInstance::update() before applying skeleton animation");

		for(int i=firstJoint; i<=lastJoint; ++i) {
			reinterpret_cast<const Quaternionf&>(result[i * trackPerJoint + Rotation]).toMatrix(tmp);
			MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(tmp.determinant(), 1, 1e-5f));

			m.setMat33(tmp);
			m.setTranslation(reinterpret_cast<const Vec3f&>(result[i * trackPerJoint + Translation]));
			pose.transforms[i] = pose.transforms[skeleton->parents[i]] * m;
		}
	}
}

void skinning(
	const StrideArray<Vec3f>& outPos,
	const StrideArray<const Vec3f>& basePosePos,
	const StrideArray<const Mat44f>& joints,
	const StrideArray<const Vec4<uint8_t> >& jointIndice,
	const StrideArray<const Vec4f>& weight)
{
	MCD_ASSERT(outPos.size == basePosePos.size);
	MCD_ASSERT(jointIndice.size == weight.size);
	MCD_ASSERT(outPos.size == weight.size);

	for(size_t i=0; i<outPos.size; ++i) {
		Vec3f p(0);
		for(size_t j=0; j<4; ++j) {
			float w = weight[i][j];
			if(w <= 0)
				break;
			size_t jointIdx = jointIndice[i][j];

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
	const StrideArray<const Vec4<uint8_t> >& jointIndice,
	const StrideArray<const Vec4f>& weight)
{
	MCD_ASSERT(outPos.size == outNormal.size);
	MCD_ASSERT(basePosePos.size == basePoseNormal.size);
	MCD_ASSERT(outPos.size == basePosePos.size);
	MCD_ASSERT(jointIndice.size == weight.size);
	MCD_ASSERT(outPos.size == weight.size);

	for(size_t i=0; i<outPos.size; ++i) {
		Vec3f p(0), n(0);
		for(size_t j=0; j<4; ++j) {
			float w = weight[i][j];
			if(w <= 0)
				break;
			size_t jointIdx = jointIndice[i][j];

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

}	// namespace MCD