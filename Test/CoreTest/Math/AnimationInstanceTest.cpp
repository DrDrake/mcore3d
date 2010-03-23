#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/AnimationInstance.h"

using namespace MCD;

TEST(AnimationInstanceTest)
{
	{	AnimationInstance a;

		CHECK_EQUAL(0u, a.trackCount());
		CHECK_EQUAL(0u, a.subtrackCount());

		CHECK(!a.getTrack(0u));
		CHECK(!a.getTrack(""));

		CHECK_EQUAL(0u, a.currentFrame());
		CHECK_EQUAL(0.0f, a.time);
	}

	{	AnimationInstance a;

		AnimationTrackPtr track1 = new AnimationTrack("track1");
		CHECK(a.addTrack(*track1, 0.5f, 1, "wtrack1"));
		AnimationTrackPtr track2 = new AnimationTrack("track2");
		CHECK(a.addTrack(*track2, 0.5f, 1, "wtrack2"));

		CHECK_EQUAL(track1, a.getTrack(0u)->track);
		CHECK_EQUAL(track2, a.getTrack(1u)->track);
		CHECK_EQUAL(track1, a.getTrack("wtrack1")->track);
		CHECK_EQUAL(track2, a.getTrack("wtrack2")->track);

		{	AnimationTrack::ScopedWriteLock lock(*track1);
			size_t tmp[] = { 3 };
			CHECK(track1->init(StrideArray<const size_t>(tmp, 1)));

			AnimationTrack::KeyFrames frames = track1->getKeyFramesForSubtrack(0);
			frames[0].time = 0;
			frames[1].time = 1;
			frames[2].time = 2;
			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
			reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);
			reinterpret_cast<Vec4f&>(frames[2]) = Vec4f(3);
		}

		{	AnimationTrack::ScopedWriteLock lock(*track2);
			size_t tmp[] = { 3 };
			CHECK(track2->init(StrideArray<const size_t>(tmp, 1)));

			AnimationTrack::KeyFrames frames = track2->getKeyFramesForSubtrack(0);
			frames[0].time = 0;
			frames[1].time = 1;
			frames[2].time = 2;
			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(3);
			reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(4);
			reinterpret_cast<Vec4f&>(frames[2]) = Vec4f(5);
		}

		{	a.time = 0;
			a.update();
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(a.weightedResult[0]);
			CHECK(pos.isNearEqual(Vec4f(2)));

			CHECK_EQUAL(0u, a.currentFrame());
		}

		{	a.time = 0.5f;
			a.update();
			const Vec4f& pos = reinterpret_cast<const Vec4f&>(a.weightedResult[0]);
			CHECK(pos.isNearEqual(Vec4f((1.5f + 3.5f)/2)));

			CHECK_EQUAL(0u, a.currentFrame());
		}

		{	a.time = 1;
			a.update();
			CHECK_EQUAL(1u, a.currentFrame());
		}

		{	a.time = 1.5f;
			a.update();
			CHECK_EQUAL(1u, a.currentFrame());
		}

		{	a.time = 2;	// Wrap back to time = 0
			a.update();
			CHECK_EQUAL(0u, a.currentFrame());
		}

		a.setTimeByFrameIndex(0);
		CHECK_EQUAL(0.0f, a.time);

		a.setTimeByFrameIndex(1);
		CHECK_EQUAL(1.0f, a.time);

		a.setTimeByFrameIndex(2);
		CHECK_EQUAL(2.0f, a.time);

		a.setTimeByFrameIndex(3);	// Out of bound gives the last frame's time.
		CHECK_EQUAL(2.0f, a.time);
	}
}
