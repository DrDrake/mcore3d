#include "Pch.h"
#include "../../MCD/Audio/AudioSource.h"
#include "../../MCD/Audio/OggLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include <fstream>

using namespace MCD;

#include "../../MCD/Audio/AudioBuffer.h"
#include "../../MCD/Core/System/StrUtility.h"

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
	OggLoader loader;
	std::ifstream is("stereo.ogg", std::ios_base::binary);

//	loader.load(&is);

	// Creating the manager and destroy it immediatly
	std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
	ResourceManager manager(*fs);
	fs.release();

	manager.addFactory(new OggLoaderFactory);
	AudioSource source;
	source.load(manager, L"stereo.ogg");

	while(true) {	// Poll for event
		ResourceManager::Event event = manager.popEvent();
		if(!event.loader) continue;

		if(event.loader->getLoadingState() != IResourceLoader::Loaded)
			CHECK(false);

		event.loader->commit(*event.resource);
		break;
	}
}
