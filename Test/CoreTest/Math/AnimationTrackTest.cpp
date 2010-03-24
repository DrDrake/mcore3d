#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationTrack.h"

using namespace MCD;

TEST(AnimationTrackTest)
{
	{	AnimationTrackPtr track = new AnimationTrack("");
	}

	{	AnimationTrackPtr track = new AnimationTrack("");
		track->acquireReadLock();
		CHECK(!track->isCommitted());
		track->releaseReadLock();

		track->acquireWriteLock();
		CHECK(!track->init(StrideArray<const size_t>(nullptr, 0)));
		track->releaseWriteLock();

		track->acquireReadLock();
		CHECK(track->isCommitted());
		CHECK_EQUAL(0u, track->keyframeCount(0));
		CHECK_EQUAL(0u, track->subtrackCount());
		track->releaseReadLock();
	}

	{	AnimationTrackPtr track = new AnimationTrack("");
		
		{	track->acquireWriteLock();
			size_t tmp[] = { 3 };
			CHECK(track->init(StrideArray<const size_t>(tmp, 1)));
			CHECK_EQUAL(3u, track->keyframeCount(0));
			CHECK_EQUAL(0u, track->keyframeCount(1));
			CHECK_EQUAL(1u, track->subtrackCount());

			tmp[0] = 2;
			CHECK(track->init(StrideArray<const size_t>(tmp, 1)));	// init() can be invoked multiple times.

			CHECK_EQUAL(2u, track->keyframeCount(0));
			CHECK_EQUAL(0u, track->keyframeCount(1));
			CHECK_EQUAL(1u, track->subtrackCount());

			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
			frames[0].time = 0;
			frames[1].time = 1;

			CHECK(track->checkValid());
			CHECK_EQUAL(AnimationTrack::Linear, track->subtracks[0].flag);
			CHECK_EQUAL(1, track->totalTime(0));

			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
			reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);

			track->releaseWriteLock();
			CHECK_EQUAL(1, track->totalTime());
		}

		AnimationTrack::Interpolation interpolation[1];
		AnimationTrack::Interpolations results(interpolation, 1);
		track->acquireReadLock();

		{	// Test for the interpolate() function
			CHECK_EQUAL(0.5f, track->interpolateNoLock(0.5f, results));
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(results[0]);
			CHECK(pos.isNearEqual(Vec4f(1.5f)));
		}

		{	// Try to play backward from time 0.5 back to 0.1
			CHECK_EQUAL(0.0f, track->interpolateNoLock(0.0f, results));
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(results[0]);
			CHECK(pos.isNearEqual(Vec4f(1)));
		}

		{	// Try to play over totalTime() under loop mode
			CHECK_EQUAL(0.0f, track->interpolateNoLock(track->totalTime() * 2, results));
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(results[0]);
			CHECK(pos.isNearEqual(Vec4f(1)));
		}

		{	// Try to play over totalTime() not under loop mode
			track->loop = false;
			CHECK_EQUAL(track->totalTime(), track->interpolateNoLock(track->totalTime() * 2, results));
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(results[0]);
			CHECK(pos.isNearEqual(Vec4f(2)));
		}

		track->releaseReadLock();
	}
}

TEST(Slerp_AnimationTrackTest)
{
	AnimationTrackPtr track = new AnimationTrack("");
	
	{	track->acquireWriteLock();
		size_t tmp[] = { 2 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].time = 0;
		frames[1].time = 1;

		track->subtracks[0].flag = AnimationTrack::Slerp;

		Quaternionf& q1 = reinterpret_cast<Quaternionf&>(frames[0]);
		Quaternionf& q2 = reinterpret_cast<Quaternionf&>(frames[1]);

		q1.fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());	// Rotate around y-axis 45 degree anti-clockwise
		q2.fromAxisAngle(Vec3f::c010, Mathf::cPi());		// Rotate around y-axis 90 degree anti-clockwise

		track->releaseWriteLock();
	}

	AnimationTrack::Interpolation interpolation[1];
	AnimationTrack::Interpolations results(interpolation, 1);
	track->acquireReadLock();
	CHECK_EQUAL(0.5f, track->interpolateNoLock(0.5f, results));
	const Quaternionf& q = reinterpret_cast<const Quaternionf&>(results[0]);

	Vec3f v;
	float angle;
	q.toAxisAngle(v, angle);
	CHECK(v.isNearEqual(Vec3f::c010));
	CHECK_CLOSE(Mathf::cPi() * 3/4, angle, 1e-6);
	track->releaseReadLock();
}

#include "../../../MCD/Core/System/Timer.h"

TEST(Performance_AnimationTrackTest)
{
	const size_t frameCount = 256;
	const size_t subtrackCount = 8;

	AnimationTrackPtr track = new AnimationTrack("");

	{	track->acquireWriteLock();
		size_t tmp[subtrackCount] = { frameCount };

		for(size_t i=0; i<subtrackCount; ++i)
			tmp[i] = frameCount;
		CHECK(track->init(StrideArray<const size_t>(tmp, subtrackCount)));

		// Making half are Liner, half are Slerp
		for(size_t i=0; i<track->subtrackCount()/2; ++i)
			track->subtracks[i].flag = AnimationTrack::Slerp;

		for(size_t i=0; i<subtrackCount; ++i) {
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<frameCount; ++j) {
				// Assign the time of each frame
				frames[j].time = float(j);
				// Fill with some random floating point data
				reinterpret_cast<Quaternionf&>(frames[j]) = Vec4f(Mathf::random(), Mathf::random(), Mathf::random(), Mathf::random());
			}
		}

		track->releaseWriteLock();
	}

	{	track->acquireReadLock();
		AnimationTrack::Interpolation interpolation[subtrackCount];
		AnimationTrack::Interpolations results(interpolation, subtrackCount);
		DeltaTimer timer;
		const float totalTime = track->totalTime();

		size_t count = 0;
		for(float t=0; t<totalTime; t+=0.1f, ++count)
			CHECK_EQUAL(t, track->interpolateNoLock(t, results));
		track->releaseReadLock();

		const double timeElasped = timer.getDelta().asSecond();
		const double attributePerSecond = double(subtrackCount) * count / timeElasped;
		(void)attributePerSecond;

		// NOTE: The variable sub-track frame count support was added at revision 733.
		// This new version does slow down the calculation by a factor of 2, but I 
		// beleive the memory saving should out perform the wasted CPU cycles.
//		std::cout << attributePerSecond;
	}
}
