#include "Pch.h"
#include "ResourceLoaderFactory.h"
#include "AudioBuffer.h"
#include "OggLoader.h"
#include "../Core/System/StrUtility.h"

namespace MCD {

ResourcePtr OggLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "ogg") != 0)
		return nullptr;

	int bufferCount = AudioBuffer::cMaxBuffers;

	if(args) {
		NvpParser parser(args);
		const char* name, *value;
		while(parser.next(name, value))
		{
			if(strCaseCmp(name, "bufferCount") == 0) {
				bufferCount = str2IntWithDefault(value, AudioBuffer::cMaxBuffers);
				break;
			}
		}

		if(bufferCount < 1)
			bufferCount = 1;
		if(bufferCount > AudioBuffer::cMaxBuffers)
			bufferCount = AudioBuffer::cMaxBuffers;
	}

	return new AudioBuffer(fileId, bufferCount);
}

IResourceLoader* OggLoaderFactory::createLoader()
{
	return new OggLoader;
}

}	// namespace MCD
