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

TEST(BasicEvent_AnimationInstanceTest)
{
	AnimationInstance::Events e;
	e.destroyData = &::free;

	CHECK(!e.getEvent(0));

	struct LocalClass {
		static void callback(const AnimationInstance::Event& e) {
		}
	};	// LocalClass

	e.setEvent(0, &LocalClass::callback, ::strdup("event1"));
	CHECK_EQUAL(std::string("event1"), (char*)e.getEvent(0)->data);

	e.setEvent(2, &LocalClass::callback, ::strdup("event2"));
	CHECK(!e.getEvent(1));
	CHECK_EQUAL(std::string("event2"), (char*)e.getEvent(2)->data);

	// Replacing old data
	e.setEvent(0, &LocalClass::callback, ::strdup("event0"));
	CHECK_EQUAL(std::string("event0"), (char*)e.getEvent(0)->data);

	// Remove data
	e.setEvent(0, nullptr, nullptr);
	CHECK(!e.getEvent(0));
}

static size_t gEventCallbackResult1 = size_t(-1);
static size_t gEventCallbackResult2 = size_t(-1);

TEST(EventCallback_AnimationInstanceTest)
{
	AnimationInstance a;
	AnimationTrackPtr track = new AnimationTrack("track");
	CHECK(a.addTrack(*track, 1, 1, "wtrack"));

	struct LocalClass {
		static void callback1(const AnimationInstance::Event& e) {
			gEventCallbackResult1 = size_t(e.data);
		}
	};	// LocalClass

	{	AnimationTrack::ScopedWriteLock lock(*track);
		size_t tmp[] = { 3 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].time = 0;
		frames[1].time = 1;
		frames[2].time = 2;
	}

	AnimationInstance::WeightedTrack& wt = *a.getTrack(0u);
	wt.edgeEvents.setEvent(0, &LocalClass::callback1, (void*)10u);
	wt.edgeEvents.setEvent(2, &LocalClass::callback1, (void*)20u);

	gEventCallbackResult1 = size_t(-1);
	gEventCallbackResult2 = size_t(-1);

	a.time = 0;
	a.update();
	CHECK_EQUAL(size_t(-1), gEventCallbackResult1);

	a.time = 0.9f;
	a.update();
	CHECK_EQUAL(size_t(-1), gEventCallbackResult1);

	a.time = 1.0f;
	a.update();
	CHECK_EQUAL(10u, gEventCallbackResult1);
}
