#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationTrack.h"

using namespace MCD;

TEST(AnimationTrackTest)
{
	{	AnimationTrack track(2, 1);

		CHECK_EQUAL(2u, track.keyframeCount());
		CHECK_EQUAL(1u, track.subtrackCount());
	}

	{	AnimationTrack track(2, 1);
		track.keyframeTimes[0] = 0;
		track.keyframeTimes[1] = 1;

		CHECK(track.checkValid());
		CHECK_EQUAL(0, track.currentTime());
		CHECK_EQUAL(AnimationTrack::Linear, track.subtrackFlags[0]);

		AnimationTrack::KeyFrames frames = track.getKeyFramesForSubtrack(0);
		reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);

		track.update(0.5f);

		const Vec4f& pos = reinterpret_cast<const Vec4f&>(track.interpolatedResult[0]);
		CHECK(pos.isNearEqual(Vec4f(1.5f)));
	}
}

TEST(Slerp_AnimationTrackTest)
{
	AnimationTrack track(2, 1);
	track.keyframeTimes[0] = 0;
	track.keyframeTimes[1] = 1;
	track.subtrackFlags[0] = AnimationTrack::Slerp;

	AnimationTrack::KeyFrames frames = track.getKeyFramesForSubtrack(0);
	Quaternionf& q1 = reinterpret_cast<Quaternionf&>(frames[0]);
	Quaternionf& q2 = reinterpret_cast<Quaternionf&>(frames[1]);

	q1.fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());	// Rotate around y-axis 45 degree anti-clockwise
	q2.fromAxisAngle(Vec3f::c010, Mathf::cPi());		// Rotate around y-axis 90 degree anti-clockwise

	track.update(0.5f);

	const Quaternionf& q = reinterpret_cast<const Quaternionf&>(track.interpolatedResult[0]);
	Vec3f v;
	float angle;
	q.toAxisAngle(v, angle);
	CHECK(v.isNearEqual(Vec3f::c010));
	CHECK_CLOSE(Mathf::cPi() * 3/4, angle, 1e-6);
}
