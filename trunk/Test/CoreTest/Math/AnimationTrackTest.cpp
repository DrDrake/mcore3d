#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationInstance.h"
#include "../../../MCD/Core/Math/AnimationTrack.h"
#include <limits>

using namespace MCD;

TEST(AnimationClipTest)
{
	{	AnimationClipPtr clip = new AnimationClip("");
	}

	{	AnimationClipPtr clip = new AnimationClip("");
		CHECK(!clip->init(StrideArray<const size_t>(nullptr, 0)));

		CHECK_EQUAL(0u, clip->trackCount());
	}

	{	AnimationClipPtr clip = new AnimationClip("");
		
		{	size_t tmp[] = { 1, 2 };
			CHECK(clip->init(StrideArray<const size_t>(tmp, 1)));
			CHECK_EQUAL(1u, clip->getKeysForTrack(0).size);
			CHECK_EQUAL(1u, clip->trackCount());

			CHECK(clip->init(StrideArray<const size_t>(tmp, 2)));	// init() can be invoked multiple times.

			CHECK_EQUAL(1u, clip->getKeysForTrack(0).size);
			CHECK_EQUAL(2u, clip->getKeysForTrack(1).size);
			CHECK_EQUAL(2u, clip->trackCount());

			AnimationClip::Keys keys0 = clip->getKeysForTrack(0);
			keys0[0].pos = 0;

			AnimationClip::Keys keys1 = clip->getKeysForTrack(1);
			keys1[0].pos = 0;
			keys1[1].pos = 1;
			clip->length = 2;

			CHECK(clip->checkValid());
			CHECK_EQUAL(AnimationClip::Linear, clip->tracks[0].flag);
			CHECK_EQUAL(0, clip->lengthForTrack(0));
			CHECK_EQUAL(1, clip->lengthForTrack(1));

			keys0[0].cast<Vec4f>() = Vec4f(1);
			keys1[0].cast<Vec4f>() = Vec4f(1);
			keys1[1].cast<Vec4f>() = Vec4f(2);
		}

		AnimationClip::Pose pose(new AnimationClip::Sample[2], 2);
		clip->interpolate(0.5f, pose);

		CHECK(pose[0].cast<Vec4f>() == Vec4f(1));
		CHECK(pose[1].cast<Vec4f>() == Vec4f(1.5f));

		delete[] pose.getPtr();
	}
}

TEST(Slerp_AnimationClipTest)
{
	AnimationClipPtr clip = new AnimationClip("");
	
	{	size_t tmp[] = { 2 };
		CHECK(clip->init(StrideArray<const size_t>(tmp, 1)));

		AnimationClip::Keys samples = clip->getKeysForTrack(0);
		samples[0].pos = 0;
		samples[1].pos = 1;

		clip->tracks[0].flag = AnimationClip::Slerp;

		Quaternionf& q1 = samples[0].cast<Quaternionf>();
		Quaternionf& q2 = samples[1].cast<Quaternionf>();

		q1.fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());	// Rotate around y-axis 45 degree anti-clockwise
		q2.fromAxisAngle(Vec3f::c010, Mathf::cPi());		// Rotate around y-axis 90 degree anti-clockwise
	}

	AnimationClip::Pose pose(new AnimationClip::Sample[1], 1);
	clip->interpolate(0.5f, pose);
	const Quaternionf& q = pose[0].cast<Quaternionf>();

	Vec3f v;
	float angle;
	q.toAxisAngle(v, angle);
	CHECK(v.isNearEqual(Vec3f::c010));
	CHECK_CLOSE(Mathf::cPi() * 3/4, angle, 1e-6);
}

TEST(AnimationStateTest)
{
	// Create a clip of length = 10
	AnimationClipPtr clip = new AnimationClip("");
	clip->length = 10 * clip->framerate;

	AnimationState a;
	a.clip = clip;

	{	// With loop
		a.worldTime = 0;
		a.worldRefTime = 0;
		a.loopCountOverride = 0;
		CHECK_EQUAL(0, a.localTime());
		CHECK_EQUAL(std::numeric_limits<float>::max(), a.worldEndTime());
		CHECK(!a.ended());	// With loop, the animation never end

		a.worldTime = 1;
		CHECK_EQUAL(1, a.localTime());

		a.worldTime = 9;
		CHECK_EQUAL(9, a.localTime());

		a.worldTime = 10;
		CHECK_EQUAL(0, a.localTime());

		a.worldTime = 11;	// Run over the clip's length
		CHECK_EQUAL(1, a.localTime());

		a.worldTime = 20;	// Run over the clip's length
		CHECK_EQUAL(0, a.localTime());

		// With rate not equals to 1
		a.rate = 2;
		a.worldTime = 1;
		CHECK_EQUAL(2, a.localTime());

		a.rate = 0.5f;
		a.worldTime = 4;
		CHECK_EQUAL(2, a.localTime());

		// Negative rate
		a.rate = -1;
		a.worldTime = 1;
		CHECK_EQUAL(9, a.localTime());
		CHECK(!a.ended());

		a.rate = -1;
		a.worldTime = 4;
		CHECK_EQUAL(6, a.localTime());

		a.rate = -1;
		a.worldTime = 11;
		CHECK_EQUAL(9, a.localTime());
	}

	{	// Without loop
		a.rate = 1;
		a.worldTime = 0;
		a.loopCountOverride = 1;
		CHECK_EQUAL(0, a.localTime());
		CHECK_EQUAL(10u, a.worldEndTime());
		CHECK(!a.ended());

		a.loopCountOverride = 2;
		CHECK_EQUAL(20u, a.worldEndTime());

		a.worldTime = 1;
		CHECK_EQUAL(1, a.localTime());

		a.worldTime = 9;
		CHECK_EQUAL(9, a.localTime());

		a.worldTime = 10;	// Eact position on end of local time line will reduce to 0 when the whole clip is not ended yet
		CHECK_EQUAL(0, a.localTime());

		a.worldTime = 20;	// For non looping animation, the end positioned in clip will result end local position
		CHECK(a.ended());
		CHECK_EQUAL(10, a.localTime());

		// With rate not equals to 1
		a.rate = 2;
		a.worldTime = 1;
		CHECK_EQUAL(2, a.localTime());
		CHECK_EQUAL(10, a.worldEndTime());

		a.rate = 0.5f;
		a.worldTime = 4;
		CHECK_EQUAL(2, a.localTime());
		CHECK_EQUAL(40, a.worldEndTime());

		// Negative rate
		a.rate = -1;
		a.worldTime = 1;
		CHECK_EQUAL(9, a.localTime());
		CHECK_EQUAL(20, a.worldEndTime());
		CHECK(!a.ended());

		a.rate = -1;
		a.worldTime = 4;
		CHECK_EQUAL(6, a.localTime());

		a.rate = -1;
		a.worldTime = 10;
		CHECK_EQUAL(10, a.localTime());

		a.rate = -1;
		a.worldTime = 11;
		CHECK_EQUAL(9, a.localTime());

		a.rate = -1;
		a.worldTime = 20;
		CHECK_EQUAL(0, a.localTime());
	}
}
