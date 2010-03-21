#include "Pch.h"
#include "../../MCD/Audio/AudioDevice.h"
#include "../../MCD/Audio/AudioSource.h"
#include "../../MCD/Audio/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Core/System/Thread.h"	// for mSleep()

using namespace MCD;

namespace {

class ThreeDTestFixture
{
protected:
	ThreeDTestFixture()
		: fs(new RawFileSystem("./")), manager(*fs)
	{
		(void)initAudioDevice();
		manager.addFactory(new OggLoaderFactory);
	}

	~ThreeDTestFixture()
	{
		closeAudioDevice();
	}

	RawFileSystem* fs;
	ResourceManager manager;
};	// ThreeDTestFixture

}	// namespace

TEST_FIXTURE(ThreeDTestFixture, LeftToRight)
{
	AudioSource source;
	CHECK(source.load(manager, "mono.ogg", "blockLoadFirstBuffer=1"));

	CHECK_EQUAL(1u, source.channelCount());
	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55799, source.totalPcm());

	source.play();
	source.setPosition(Vec3f(-10, 0, 0));

	while(source.isPlaying()) {
		mSleep(20);
		source.update();
		source.setPosition(source.position() + Vec3f(0.2f, 0, 0));
		manager.popEvent();
	}

	CHECK_EQUAL(source.totalPcm(), source.currentPcm());
}
