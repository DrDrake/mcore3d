#ifndef __MCD_AUDIO_RESOURCELOADERFACTORY__
#define __MCD_AUDIO_RESOURCELOADERFACTORY__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

class MCD_AUDIO_API OggLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoader* createLoader();
};	// OggLoaderFactory

}	// namespace MCD

#endif	// __MCD_AUDIO_RESOURCELOADERFACTORY__
