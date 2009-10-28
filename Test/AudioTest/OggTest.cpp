#include "Pch.h"
#include "../../MCD/Audio/AudioSource.h"
#include "../../MCD/Audio/OggLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include <fstream>

using namespace MCD;

#include "../../MCD/Audio/AudioBuffer.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Core/System/Thread.h"

class OggLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args) {
		if(wstrCaseCmp(fileId.getExtension().c_str(), L"ogg") == 0)
			return new AudioBuffer(fileId, AudioBuffer::cMaxBuffers);
		return nullptr;
	}

	sal_override IResourceLoader* createLoader() {
		return new OggLoader;
	}
};	// OggFactory

class OggTestFixture
{
protected:
	OggTestFixture()
		: manager(*(new RawFileSystem(L"./")))
	{
		initAudio();
		manager.addFactory(new OggLoaderFactory);
	}

	void waitForSourceFinish(AudioSource& source)
	{
		while(source.isReallyPlaying()) {
			mSleep(1);
			source.update();
			ResourceManager::Event event = manager.popEvent();
			if(!event.loader) continue;

			event.loader->commit(*event.resource);
		}
	}

	ResourceManager manager;
};	// OggTestFixture

// This first-block mode is convenience to the user for getting the source's property,
// and have exact timming on when to play the source.
TEST_FIXTURE(OggTestFixture, aOggStreamBlockFirstPartialTest)
{
	AudioSource source;
	CHECK(source.load(manager, L"stereo.ogg", true));

	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());

	// The source start to play instantly at this call.
	source.play();

	waitForSourceFinish(source);
}

// Most of the cases this non-block mode is used, which should gives shortest loading time.
// The draw back is that it's harder to sure when the source is actually played.
TEST_FIXTURE(OggTestFixture, cOggStreamNonBlock)
{
	AudioSource source;
	CHECK(source.load(manager, L"stereo.ogg", false));

	// Calling play() just inform the source to play when data is ready.
	source.play();

	// Ensure the source has start to play
	while(!source.isReallyPlaying()) {
		source.update();
		manager.popEvent();
	}

	// Then we can examin the properties.
	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());

	waitForSourceFinish(source);
}

#include "../../MCD/Audio/AudioEffect.h"

// TODO: Move the effect test to somewhere else
TEST_FIXTURE(OggTestFixture, bEffect)
{
	initAudioEffect();

	AudioSource source;
	CHECK(source.load(manager, L"stereo.ogg", true));

	AudioEffect effect;
	effect.create();

	effect.bind(source);
	source.play();

	waitForSourceFinish(source);
}
