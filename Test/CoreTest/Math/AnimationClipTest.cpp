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
		clip->sample(0.5f, pose);

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

		AnimationClip::Keys keys = clip->getKeysForTrack(0);
		keys[0].pos = 0;
		keys[1].pos = 1;

		clip->tracks[0].flag = AnimationClip::Slerp;

		// Rotate around y-axis 45 degree anti-clockwise
		keys[0].cast<Quaternionf>() = Quaternionf::makeAxisAngle(Vec3f::c010, Mathf::cPiOver2());
		// Rotate around y-axis 90 degree anti-clockwise
		keys[1].cast<Quaternionf>() = Quaternionf::makeAxisAngle(Vec3f::c010, Mathf::cPi());
	}

	AnimationClip::Pose pose(new AnimationClip::Sample[1], 1);
	clip->sample(0.5f, pose);
	const Quaternionf& q = pose[0].cast<Quaternionf>();

	Vec3f v;
	float angle;
	q.toAxisAngle(v, angle);
	CHECK(v.isNearEqual(Vec3f::c010));
	CHECK_CLOSE(Mathf::cPi() * 3/4, angle, 1e-6);

	delete[] pose.getPtr();
}

TEST(Difference_AnimationClipTest)
{
	AnimationClipPtr masterClip = new AnimationClip("");
	AnimationClipPtr targetClip = new AnimationClip("");

	{	// Create the master clip, which has a constant y,z-direction translation
		// and constant rotation around y-axis of 45 degree
		size_t tmp[] = { 1, 1 };
		CHECK(masterClip->init(StrideArray<const size_t>(tmp, 2)));

		masterClip->tracks[0].flag = AnimationClip::Linear;
		masterClip->tracks[1].flag = AnimationClip::Slerp;

		AnimationClip::Keys keys = masterClip->getKeysForTrack(0);
		keys[0].pos = 0;
		keys[0].v = Vec4f(0, 1, 1, 0);

		keys = masterClip->getKeysForTrack(1);
		keys[0].pos = 0;
		keys[0].cast<Quaternionf>().fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());
	}

	{	// Create the target clip, which has a constant z-direction translation
		// and constant rotation around y-axis of 45 degree
		size_t tmp[] = { 2, 2 };
		CHECK(targetClip->init(StrideArray<const size_t>(tmp, 2)));

		targetClip->tracks[0].flag = AnimationClip::Linear;
		targetClip->tracks[1].flag = AnimationClip::Slerp;

		AnimationClip::Keys keys = targetClip->getKeysForTrack(0);
		keys[0].pos = 0;
		keys[0].v = Vec4f(0, 0, 1, 0);
		keys[1].pos = targetClip->framerate;
		keys[1].v = Vec4f(2, 2, 1, 0);

		keys = targetClip->getKeysForTrack(1);
		keys[0].pos = 0;
		keys[0].cast<Quaternionf>() = Quaternionf::cIdentity;
		keys[1].pos = targetClip->framerate;
		keys[1].cast<Quaternionf>().fromAxisAngle(Vec3f::c010, Mathf::cPi());
	}

	AnimationClipPtr diffClip = new AnimationClip("");

	CHECK(diffClip->createDifferenceClip(*masterClip, *targetClip));

	// Check the difference clip is correct
	CHECK_EQUAL(targetClip->length, diffClip->length);
	CHECK_EQUAL(targetClip->framerate, diffClip->framerate);
	CHECK_EQUAL(targetClip->loopCount, diffClip->loopCount);

	AnimationClip::Pose pose(new AnimationClip::Sample[2], 2);

	diffClip->sample(0, pose);
	CHECK(Vec4f(0, -1, 0, 0) == pose[0].v);
	CHECK(Quaternionf::makeAxisAngle(Vec3f::c010, -Mathf::cPiOver2()).isNearEqual(pose[1].cast<Quaternionf>()));

	diffClip->sample(diffClip->framerate, pose);
	CHECK(Vec4f(2, 1, 0, 0) == pose[0].v);
	CHECK(Quaternionf::makeAxisAngle(Vec3f::c010, Mathf::cPiOver2()).isNearEqual(pose[1].cast<Quaternionf>()));

	delete[] pose.getPtr();
}
