#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/AnimationState.h"
#include <limits>

using namespace MCD;

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

TEST(AnimationInstanceTest)
{
/*	{	AnimationInstance a;

		CHECK_EQUAL(0u, a.trackCount());
		CHECK_EQUAL(0u, a.subtrackCount());

		CHECK(!a.getTrack(0u));
		CHECK(!a.getTrack(""));

		CHECK_EQUAL(0u, a.currentFrame());
		CHECK_EQUAL(0.0f, a.time);
	}

	{	AnimationInstance a;

		AnimationClipPtr track1 = new AnimationClip("track1");
		CHECK(a.addTrack(*track1, 0.5f, 1, "wtrack1"));
		AnimationClipPtr track2 = new AnimationClip("track2");
		CHECK(a.addTrack(*track2, 0.5f, 1, "wtrack2"));

		CHECK_EQUAL(track1, a.getTrack(0u)->track);
		CHECK_EQUAL(track2, a.getTrack(1u)->track);
		CHECK_EQUAL(track1, a.getTrack("wtrack1")->track);
		CHECK_EQUAL(track2, a.getTrack("wtrack2")->track);

		{	AnimationClip::ScopedWriteLock lock(*track1);
			size_t tmp[] = { 3 };
			CHECK(track1->init(StrideArray<const size_t>(tmp, 1)));

			AnimationClip::KeyFrames frames = track1->getKeyFramesForSubtrack(0);
			frames[0].pos = 0;
			frames[1].pos = 1;
			frames[2].pos = 2;
			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
			reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);
			reinterpret_cast<Vec4f&>(frames[2]) = Vec4f(3);
		}

		{	AnimationClip::ScopedWriteLock lock(*track2);
			size_t tmp[] = { 3 };
			CHECK(track2->init(StrideArray<const size_t>(tmp, 1)));

			AnimationClip::KeyFrames frames = track2->getKeyFramesForSubtrack(0);
			frames[0].pos = 0;
			frames[1].pos = 1;
			frames[2].pos = 2;
			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(3);
			reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(4);
			reinterpret_cast<Vec4f&>(frames[2]) = Vec4f(5);
		}

		CHECK_EQUAL(2, a.length());
		CHECK_EQUAL(2, a.totalTime());

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
	AnimationClipPtr track = new AnimationClip("track");
	CHECK(a.addTrack(*track, 1, 1, "wtrack"));

	{	AnimationClip::ScopedWriteLock lock(*track);
		size_t tmp[] = { 3 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationClip::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].pos = 0;
		frames[1].pos = 1;
		frames[2].pos = 2;
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
	CHECK_EQUAL(20u, gEventCallbackResult1[3]);*/
}
