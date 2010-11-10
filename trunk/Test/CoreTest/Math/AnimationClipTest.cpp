#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationClip.h"

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
