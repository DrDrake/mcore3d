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
	CHECK(!e.getEvent(0));

	struct LocalClass {
		static void callback(const AnimationInstance::Event& e) {
		}
	};	// LocalClass

	e.callback = &LocalClass::callback;
	e.destroyData = &::free;

	CHECK(e.setEvent(0, ::strdup("event1")));
	if(AnimationInstance::Event* ev = e.getEvent(0)) {
		CHECK_EQUAL(std::string("event1"), (char*)ev->data);
	} else
		CHECK(false);

	CHECK(e.setEvent(2, ::strdup("event2")));
	CHECK(!e.getEvent(1));
	if(AnimationInstance::Event* ev = e.getEvent(2)) {
		CHECK_EQUAL(std::string("event2"), (char*)ev->data);
	} else
		CHECK(false);

	// Replacing old data
	CHECK(e.setEvent(0, ::strdup("event0")));
	if(AnimationInstance::Event* ev = e.getEvent(0)) {
		CHECK_EQUAL(std::string("event0"), (char*)ev->data);
	} else
		CHECK(false);

	// Remove data
	CHECK(!e.setEvent(0, nullptr));
	CHECK(!e.getEvent(0));
}

static std::vector<size_t> gEventCallbackResult1, gEventCallbackResult2;

TEST(EventCallback_AnimationInstanceTest)
{
	AnimationInstance a;
	AnimationTrackPtr track = new AnimationTrack("track");
	CHECK(a.addTrack(*track, 1, 1, "wtrack"));

	{	AnimationTrack::ScopedWriteLock lock(*track);
		size_t tmp[] = { 3 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].time = 0;
		frames[1].time = 1;
		frames[2].time = 2;
	}

	struct LocalClass {
		static void callback1(const AnimationInstance::Event& e) {
			gEventCallbackResult1.push_back(size_t(e.data));
		}
		static void callback2(const AnimationInstance::Event& e) {
			gEventCallbackResult2.push_back(size_t(e.data));
		}
	};	// LocalClass

	AnimationInstance::WeightedTrack& wt = *a.getTrack(0u);

	wt.edgeEvents.callback = &LocalClass::callback1;
	CHECK(wt.edgeEvents.setEvent(0, (void*)10u));
	CHECK(wt.edgeEvents.setEvent(2, (void*)20u));

	wt.levelEvents.callback = &LocalClass::callback2;
	CHECK(wt.levelEvents.setEvent(0, (void*)10u));
	CHECK(wt.levelEvents.setEvent(2, (void*)20u));

	gEventCallbackResult1.clear();
	gEventCallbackResult2.clear();

	// Got both edge and level event
	a.time = 0;
	a.update();
	CHECK_EQUAL(10u, gEventCallbackResult1[0]);
	CHECK_EQUAL(10u, gEventCallbackResult2[0]);

	// Still in the time of the first frame, no edge event but a level event
	a.time = 0.9f;
	a.update();
	CHECK_EQUAL(1u, gEventCallbackResult1.size());
	CHECK_EQUAL(10u, gEventCallbackResult2[1]);

	// No event for both edge and level
	a.time = 1.0f;
	a.update();
	CHECK_EQUAL(1u, gEventCallbackResult1.size());
	CHECK_EQUAL(2u, gEventCallbackResult2.size());

	// 2 events for looped time edge event, and one level event
	a.time = 2.2f;
	a.update();
	CHECK_EQUAL(20u, gEventCallbackResult1[1]);
	CHECK_EQUAL(10u, gEventCallbackResult1[2]);
	CHECK_EQUAL(10u, gEventCallbackResult2[2]);

	// Disable looping, and test for the last frame. Both edge and level should got event
	wt.loopOverride = 0;
	a.update();
	CHECK_EQUAL(20u, gEventCallbackResult1[3]);
	CHECK_EQUAL(20u, gEventCallbackResult1[3]);
}
