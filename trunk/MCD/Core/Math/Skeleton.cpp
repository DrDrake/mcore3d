#include "Pch.h"
#include "Skeleton.h"
#include "Quaternion.h"

namespace MCD {

void Skeleton::init(size_t jointCount)
{
	transforms.resize(jointCount, Mat44f::cIdentity);
	parents.resize(jointCount);
}

void SkeletonAnimation::applyTo(Skeleton& skeleton, int firstJoint, int lastJoint)
{
	if(firstJoint < 0) firstJoint = 0;
	if(lastJoint < 0) lastJoint = skeleton.transforms.size() - 1;

	MCD_ASSERT(firstJoint < lastJoint);
	MCD_ASSERT(lastJoint < int(skeleton.transforms.size()));

	size_t trackPerJoint = anim.subtrackCount() / skeleton.transforms.size();
	if(trackPerJoint < 0)
		return;

	if(trackPerJoint == 2)	// Translation and rotation
	{
		Mat44f m = Mat44f::cIdentity;
		Mat33f tmp;

		// NOTE: Accessing interpolatedResult will cause race condition with animation thread,
		// but it's fine for smoothing varing data.
		const AnimationTrack::KeyFrames& result = anim.interpolatedResult;

		MCD_ASSERT(result.data && "Call AnimationInstance::update() before applying skeleton animation");

		for(int i=firstJoint; i<=lastJoint; ++i) {
			reinterpret_cast<const Quaternionf&>(result[i * trackPerJoint + Rotation]).toMatrix(tmp);
			MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(tmp.determinant(), 1, 1e-5f));

			m.setMat33(tmp);
			m.setTranslation(reinterpret_cast<const Vec3f&>(result[i * trackPerJoint + Translation]));
			skeleton.transforms[i] = skeleton.transforms[skeleton.parents[i]] * m;
		}
	}
}

}	// namespace MCD
