#include "Pch.h"
#include "../../../MCD/Core/Math/Skeleton.h"
#include "../../../MCD/Core/Math/Quaternion.h"

using namespace MCD;

TEST(SkeletonTest)
{
	// We going to create the following skeleton structure
	// Bone01 [0, 0.5, 0]
	// |-Bone02 [0, 1.5, 0]
	//   |-Bone03 [0, 2.5, 0]
	// And an animation that will rotate each joint around z-axis
	AnimationTrackPtr track = new AnimationTrack(L"");

	static const size_t jointCount = 3;
	static const size_t frameCount = 2;

	{	track->acquireWriteLock();

		// Number of tracks = number of joint * attribute count (which is 2 because of translation and rotation)
		CHECK(track->init(frameCount, jointCount * 2));

		track->keyframeTimes[0] = 0;
		track->keyframeTimes[1] = 1;

		// Setting up the transform for each joint relative to it's parent joint.
		for(size_t i=0; i<track->subtrackCount(); ++i) {
			// Setup translation
			if(i % 2 == 0) {
				track->subtrackFlags[i] = AnimationTrack::Linear;
				for(size_t j=0; j<track->keyframeCount(); ++j) {
					AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);
					reinterpret_cast<Vec3f&>(frames[j]) = Vec3f(0, i == 0 ? 0.5f : 1, 0);
				}
			}
			// Setup rotation
			else {
				track->subtrackFlags[i] = AnimationTrack::Slerp;
				for(size_t j=0; j<track->keyframeCount(); ++j) {
					AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);
					Quaternionf& q = reinterpret_cast<Quaternionf&>(frames[j]);
					q.fromAxisAngle(Vec3f::c001, Mathf::cPi() * j / 10);	// Rotate around z-axis anti-clockwise for each key frame
				}
			}
		}

		track->releaseWriteLock();
	}

	Skeleton skeleton;
	skeleton.init(jointCount);

	skeleton.parents[0] = 0;
	skeleton.parents[1] = 0;
	skeleton.parents[2] = 1;

	SkeletonAnimation skAnimation;
	skAnimation.anim.addTrack(*track, 1);

	skAnimation.anim.time = 0.0f;
	skAnimation.anim.update();
	skeleton.rootJointTransform() = Mat44f::cIdentity;
	skeleton.rootJointTransform().translateBy(Vec3f::c001);	// Set the skeleton's object transform
	skAnimation.applyTo(skeleton);

	CHECK(skeleton.transforms[0].translation() == Vec3f(0, 0.5f, 1));
	CHECK(skeleton.transforms[1].translation() == Vec3f(0, 1.5f, 1));
	CHECK(skeleton.transforms[2].translation() == Vec3f(0, 2.5f, 1));

	skAnimation.anim.time = 0.5f;
	skAnimation.anim.update();
	skeleton.rootJointTransform() = Mat44f::cIdentity;
	skeleton.rootJointTransform().translateBy(Vec3f(0, 0, 2));	// Set the skeleton's object transform
	skAnimation.applyTo(skeleton);

	CHECK(skeleton.transforms[0].translation() == Vec3f(0, 0.5f, 2));
	CHECK(skeleton.transforms[1].translation().isNearEqual(Vec3f(-0.15643446f, 1.4876883f, 2)));
	CHECK(skeleton.transforms[2].translation().isNearEqual(Vec3f(-0.46545148f, 2.4387448f, 2)));
}
