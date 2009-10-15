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

TEST(OggTest)
{
	initAudio();

	std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
	ResourceManager manager(*fs);
	fs.release();

	manager.addFactory(new OggLoaderFactory);
	AudioSource source;
	CHECK(source.load(manager, L"stereo.ogg"));
//	source.load(manager, L"BaseSound.ogg");

	source.play();

	// Ensure the source has start to play
	while(!source.isPlaying()) {
		source.update();
		manager.popEvent();
	}

	while(source.isPlaying()) {
		mSleep(1);
		source.update();
		ResourceManager::Event event = manager.popEvent();
		if(!event.loader) continue;

		event.loader->commit(*event.resource);
	}
}
