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

	ResourceManager manager;
};	// OggTestFixture

TEST_FIXTURE(OggTestFixture, OggStreamBlockFirstPartialTest)
{
	AudioSource source;
	CHECK(source.load(manager, L"stereo.ogg", true));
//	source.load(manager, L"BaseSound.ogg");

	CHECK_EQUAL(22050u, source.frequency());
	CHECK_EQUAL(55167u, source.totalPcm());

	source.play();

	// Ensure the source has start to play
	while(!source.isReallyPlaying()) {
		source.update();
		manager.popEvent();
	}

	while(source.isReallyPlaying()) {
		mSleep(1);
		source.update();
		ResourceManager::Event event = manager.popEvent();
		if(!event.loader) continue;

		event.loader->commit(*event.resource);
	}
}
