#include "Pch.h"
#include "../../MCD/Audio/AudioDevice.h"
#include "../../MCD/Audio/AudioSource.h"
#include "../../MCD/Audio/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Core/System/TaskPool.h"

using namespace MCD;

namespace {

class OggTestFixture
{
protected:
	OggTestFixture()
		: fs(new RawFileSystem("./")), manager(*fs)
	{
		(void)initAudioDevice();
		manager.addFactory(new OggLoaderFactory);
		manager.taskPool().setThreadCount(1);	// For some tests we have use thread loading
	}

	~OggTestFixture()
	{
		closeAudioDevice();
	}

	void waitForSourceFinish(AudioSource& source)
	{
		while(source.isPlaying()) {
			mSleep(100);
			source.update();
			manager.popEvent();
		}
	}

	RawFileSystem* fs;
	ResourceManager manager;
};	// OggTestFixture

}	// namespace

TEST_FIXTURE(OggTestFixture, BasicTest)
{
	{	// Construct and destroy
		AudioSource source;
	}

	{	// Construct, load and destroy
		AudioSource source;
		CHECK(source.load(manager, "stereo.ogg"));
		CHECK_EQUAL("stereo.ogg", source.fileId().getString());
		CHECK_EQUAL(&manager, source.resourceManager());
	}
}

// This first-block mode is convenience to the user for getting the source's property,
// and have exact timming on when to play the source.
TEST_FIXTURE(OggTestFixture, StreamBlockFirstPartialTest)
{
	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg", "blockLoadFirstBuffer=1"));

	CHECK_EQUAL("blockLoadFirstBuffer=1", source.loadOptions());
	CHECK_EQUAL(2u, source.channelCount());
	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());
	CHECK_EQUAL(0u, source.currentPcm());
	CHECK_EQUAL(1, source.gain());

	// The source start to play instantly at this call.
	source.play();

	waitForSourceFinish(source);

	CHECK_EQUAL(source.totalPcm(), source.currentPcm());
}

// Test the case when using only a sinlge sub-buffer.
// For a single sub-buffer source, the buffer length should be large enough to store the whole audio.
TEST_FIXTURE(OggTestFixture, SingleSubBuffer)
{
	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg", "blockLoadFirstBuffer=1;bufferCount=1;subBufferLength=2600"));

	CHECK_EQUAL(2u, source.channelCount());
	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());
	CHECK_EQUAL(0u, source.currentPcm());

	// The source start to play instantly at this call.
	source.play();

	waitForSourceFinish(source);

	CHECK_EQUAL(source.totalPcm(), source.currentPcm());
}

// Most of the cases this non-block mode is used, which should gives shortest loading time.
// The draw back is that it's harder to sure when the source is actually played.
TEST_FIXTURE(OggTestFixture, StreamNonBlockTest)
{
	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg"));

	// Calling play() just inform the source to play when data is ready.
	source.play();

	// Ensure the source has start to play
	while(!source.isReallyPlaying()) {
		source.update();
		manager.popEvent();
	}

	// Then we can examin the properties.
	CHECK_EQUAL(2u, source.channelCount());
	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());

	waitForSourceFinish(source);

	CHECK_EQUAL(source.totalPcm(), source.currentPcm());
}

TEST_FIXTURE(OggTestFixture, SeekingTest)
{
	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg"));

	source.play();

	int count = 0;
	while(source.isPlaying() && count < 40) {
		source.update();
		ResourceManager::Event event = manager.popEvent();

		if(event.loader)
			event.loader->commit(*event.resource);

		mSleep(100);

		// Seek to half of the current PCM every 1 second
		if(count++ % 10 == 0) {
			uint64_t backupPcm = source.currentPcm();
			CHECK(source.seek(source.currentPcm() / 2));
			CHECK_EQUAL(backupPcm / 2, source.currentPcm());
		}
	}
}

TEST_FIXTURE(OggTestFixture, LoopTest)
{
	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg"));

	source.play();
	waitForSourceFinish(source);

	CHECK(source.seek(0));
	source.play();
	waitForSourceFinish(source);
}

#include "../../MCD/Core/System/Timer.h"

TEST_FIXTURE(OggTestFixture, MultiSourceTest)
{
	AudioSource source1, source2;
	CHECK(source1.load(manager, "stereo.ogg"));
	CHECK(source2.load(manager, "stereo.ogg"));

	source1.play();
	Timer timer;

	while(source1.isPlaying() || source2.isPlaying()) {
		if(timer.get().asSecond() > 1 && !source2.isPlaying())
			source2.play();

		manager.popEvent();
		source1.update();
		source2.update();
	}
}

#include "../../MCD/Audio/AudioEffect.h"

// TODO: Move the effect test to somewhere else
TEST_FIXTURE(OggTestFixture, EffectTest)
{
	initAudioEffect();

	AudioSource source;
	CHECK(source.load(manager, "stereo.ogg", "blockLoadFirstBuffer=1"));

	AudioEffect effect;
	effect.create();

	effect.bind(source);
	source.play();

	waitForSourceFinish(source);
}
