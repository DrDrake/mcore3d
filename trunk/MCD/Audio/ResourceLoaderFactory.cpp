#include "Pch.h"
#include "ResourceLoaderFactory.h"
#include "AudioBuffer.h"
#include "OggLoader.h"
#include "../Core/System/StrUtility.h"

namespace MCD {

ResourcePtr OggLoaderFactory::createResource(const Path& fileId, const wchar_t* args)
{
	int bufferCount = AudioBuffer::cMaxBuffers;

	if(args) {
		NvpParser parser(args);
		const wchar_t* name, *value;
		while(parser.next(name, value))
		{
			if(wstrCaseCmp(name, L"bufferCount") == 0) {
				bufferCount = wStr2IntWithDefault(value, AudioBuffer::cMaxBuffers);
				break;
			}
		}

		if(bufferCount < 1)
			bufferCount = 1;
		if(bufferCount > AudioBuffer::cMaxBuffers)
			bufferCount = AudioBuffer::cMaxBuffers;
	}

	if(wstrCaseCmp(fileId.getExtension().c_str(), L"ogg") == 0)
		return new AudioBuffer(fileId, bufferCount);
	return nullptr;
}

IResourceLoader* OggLoaderFactory::createLoader()
{
	return new OggLoader;
}

}	// namespace MCD
