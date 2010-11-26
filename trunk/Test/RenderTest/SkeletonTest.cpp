#include "Pch.h"
#include "../../MCD/Render/Skeleton.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/Quaternion.h"

using namespace MCD;

TEST(SkeletonTest)
{
	Entity e("Skeleton");

	// Create skeleton
	static const size_t jointCount = 3;
	SkeletonPosePtr pose = e.addComponent(new SkeletonPose);
	
	{	SkeletonPtr skeleton = new Skeleton("");
		pose->skeleton = skeleton;
		skeleton->init(jointCount);

		skeleton->parents[0] = 0;
		skeleton->parents[1] = 0;
		skeleton->parents[2] = 1;

		CHECK_EQUAL(jointCount, skeleton->basePose.size());
		CHECK_EQUAL(jointCount, skeleton->basePoseInverse.size());
	}

	// Create animation
	SimpleAnimationComponent* anim = e.addComponent(new SimpleAnimationComponent);

	{	// We going to create the following skeleton structure
		// Bone01 [0, 0.5, 0]
		// |-Bone02 [0, 1.5, 0]
		//   |-Bone03 [0, 2.5, 0]
		// And an animation that will rotate each joint around z-axis
		AnimationClipPtr clip = new AnimationClip("");

		static const size_t sampleCount = 2;
		static const size_t trackCount = jointCount * 2;

		// Create animation clip
		// Number of tracks = number of joint * attribute count (which is 2 because of translation and rotation)
		std::vector<size_t> tmp(trackCount, sampleCount);
		CHECK(clip->init(StrideArray<const size_t>(&tmp[0], trackCount)));
		clip->length = 2 * clip->framerate;

		// Setting up the transform for each joint relative to it's parent joint.
		for(size_t i=0; i<clip->trackCount(); ++i) {
			AnimationClip::Keys keys = clip->getKeysForTrack(i);

			// Setup frame position
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j) * clip->framerate;

			// Setup translation
			if(i % 2 == 0) {
				clip->tracks[i].flag = AnimationClip::Linear;
				for(size_t j=0; j<keys.size; ++j)
					keys[j].cast<Vec3f>() = Vec3f(0, i == 0 ? 0.5f : 1, 0);
			}
			// Setup rotation
			else {
				clip->tracks[i].flag = AnimationClip::Slerp;
				for(size_t j=0; j<keys.size; ++j)
					keys[j].cast<Quaternionf>().fromAxisAngle(Vec3f::c001, Mathf::cPi() * j / 10);	// Rotate around z-axis anti-clockwise for each key frame
			}
		}

		{	AnimationBlendTree::ClipNode* n = new AnimationBlendTree::ClipNode;
			n->state.clip = clip;
			anim->blendTree.nodes.push_back(n);
		}
		pose->animation = anim;

		anim->update(0);	// Force all the member variable to be evaluated
	}

	pose->transforms.resize(jointCount);
	pose->transforms[0] = Mat44f::makeTranslation(Vec3f::c001);	// Set the skeleton pose's object transform
	pose->update();	// Force all the member variable to be evaluated

	Vec3f v = pose->transforms[2].translation();
	CHECK(pose->transforms[0].translation() == Vec3f(0, 0.5f, 1));
	CHECK(pose->transforms[1].translation() == Vec3f(0, 1.5f, 1));
	CHECK(pose->transforms[2].translation() == Vec3f(0, 2.5f, 1));

	anim->update(0.5f);
	pose->transforms[0] = Mat44f::makeTranslation(Vec3f(0, 0, 2));	// Set the skeleton pose's object transform
	pose->update();

	CHECK(pose->transforms[0].translation() == Vec3f(0, 0.5f, 2));
	CHECK(pose->transforms[1].translation().isNearEqual(Vec3f(-0.15643446f, 1.4876883f, 2)));
	CHECK(pose->transforms[2].translation().isNearEqual(Vec3f(-0.46545148f, 2.4387448f, 2)));
}
