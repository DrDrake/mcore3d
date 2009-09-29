#include "Pch.h"
#include "AudioBuffer.h"
#include "../../3Party/OpenAL/al.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenAL32")
#endif

namespace MCD {

AudioBuffer::AudioBuffer(const Path& fileId)
	: Resource(fileId)
{
	alGenSources(1, &handle);
}

AudioBuffer::~AudioBuffer()
{
	alDeleteBuffers(1, &handle);
}

}	// namespace MCD
