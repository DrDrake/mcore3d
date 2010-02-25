#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationTrack.h"
#include "../../../MCD/Core/Math/AnimationTrackWriter.h"
#include "../../../MCD/Core/Math/AnimationTrackLoader.h"
#include <fstream>

using namespace MCD;

TEST(AnimationTrackWriterLoaderTest)
{
	AnimationTrackPtr track = new AnimationTrack(L"");
	
	{	// Create the source track
		AnimationTrack::ScopedWriteLock lock(*track);

		size_t tmp[] = { 3 };
		CHECK(track->init(StrideArray<const size_t>(tmp, 1)));

		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		frames[0].time = 0;
		frames[1].time = 1;
		frames[2].time = 2;
		track->loop = false;

		CHECK(track->checkValid());

		reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);
		reinterpret_cast<Vec4f&>(frames[2]) = Vec4f(3);
	}

	{	// Save the track
		std::ofstream os("TestData/tmp.anim", std::ios::binary);
		CHECK(AnimationTrackWriter::write(os, *track));
	}

	{	// Load the track
		AnimationTrackLoader loader;
		std::ifstream is("TestData/tmp.anim", std::ios::binary);
		CHECK_EQUAL(IResourceLoader::Loaded, loader.load(&is, nullptr));

		AnimationTrackPtr track2 = new AnimationTrack(L"");
		loader.commit(*track2);

		// Check the result
		AnimationTrack::ScopedReadLock lock(*track), lock2(*track2);

		CHECK_EQUAL(track->loop, track2->loop);
		CHECK_EQUAL(track->totalTime(), track2->totalTime());
		CHECK_EQUAL(track->subtrackCount(), track2->subtrackCount());
		CHECK_EQUAL(track->naturalFramerate, track2->naturalFramerate);

		for(size_t i=0; i<track->subtrackCount(); ++i) {
			CHECK_EQUAL(track->totalTime(i), track2->totalTime(i));
			CHECK_EQUAL(track->keyframeCount(i), track2->keyframeCount(i));

			AnimationTrack::KeyFrames f1 = track->getKeyFramesForSubtrack(i);
			AnimationTrack::KeyFrames f2 = track2->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<f1.size; ++j) {
				CHECK_EQUAL(f1[j].time, f2[j].time);
				CHECK(reinterpret_cast<Vec4f&>(f1[j].v) == reinterpret_cast<Vec4f&>(f2[j].v));
			}
		}
	}
}
