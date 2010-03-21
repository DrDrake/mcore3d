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
	AnimationTrackPtr track = new AnimationTrack("");

	static const size_t jointCount = 3;
	static const size_t frameCount = 2;
	static const size_t subtrackCount = jointCount * 2;

	{	AnimationTrack::ScopedWriteLock lock(*track);

		// Number of tracks = number of joint * attribute count (which is 2 because of translation and rotation)
		std::vector<size_t> tmp(subtrackCount, frameCount);
		CHECK(track->init(StrideArray<const size_t>(&tmp[0], subtrackCount)));

		// Setting up the transform for each joint relative to it's parent joint.
		for(size_t i=0; i<track->subtrackCount(); ++i) {
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);

			// Setup time
			for(size_t j=0; j<frames.size; ++j)
				frames[j].time = float(j);

			// Setup translation
			if(i % 2 == 0) {
				track->subtracks[i].flag = AnimationTrack::Linear;
				for(size_t j=0; j<frames.size; ++j) {
					reinterpret_cast<Vec3f&>(frames[j]) = Vec3f(0, i == 0 ? 0.5f : 1, 0);
				}
			}
			// Setup rotation
			else {
				track->subtracks[i].flag = AnimationTrack::Slerp;
				for(size_t j=0; j<frames.size; ++j) {
					Quaternionf& q = reinterpret_cast<Quaternionf&>(frames[j]);
					q.fromAxisAngle(Vec3f::c001, Mathf::cPi() * j / 10);	// Rotate around z-axis anti-clockwise for each key frame
				}
			}
		}
	}

	SkeletonPtr skeleton = new Skeleton("");
	skeleton->init(jointCount);

	CHECK_EQUAL(jointCount, skeleton->basePose.jointCount());
	CHECK_EQUAL(jointCount, skeleton->basePoseInverse.size());

	skeleton->parents[0] = 0;
	skeleton->parents[1] = 0;
	skeleton->parents[2] = 1;

	SkeletonAnimationPtr skAnimation = new SkeletonAnimation("");
	skAnimation->skeleton = skeleton;

	CHECK(skAnimation->anim.addTrack(*track, 1));
	skAnimation->anim.time = 0.0f;
	skAnimation->anim.update();

	SkeletonPose pose;
	pose.init(jointCount);
	pose.rootJointTransform() = Mat44f::cIdentity;
	pose.rootJointTransform().translateBy(Vec3f::c001);	// Set the skeleton pose's object transform
	skAnimation->applyTo(pose);

	CHECK(pose.transforms[0].translation() == Vec3f(0, 0.5f, 1));
	CHECK(pose.transforms[1].translation() == Vec3f(0, 1.5f, 1));
	CHECK(pose.transforms[2].translation() == Vec3f(0, 2.5f, 1));

	skAnimation->anim.time = 0.5f;
	skAnimation->anim.update();
	pose.rootJointTransform() = Mat44f::cIdentity;
	pose.rootJointTransform().translateBy(Vec3f(0, 0, 2));	// Set the skeleton's object transform
	skAnimation->applyTo(pose);

	CHECK(pose.transforms[0].translation() == Vec3f(0, 0.5f, 2));
	CHECK(pose.transforms[1].translation().isNearEqual(Vec3f(-0.15643446f, 1.4876883f, 2)));
	CHECK(pose.transforms[2].translation().isNearEqual(Vec3f(-0.46545148f, 2.4387448f, 2)));
}
