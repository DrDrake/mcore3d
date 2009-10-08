#include "Pch.h"
#include "AudioBuffer.h"
#include "../Core/System/Utility.h"
#include "../../3Party/OpenAL/al.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenAL32")
#endif

namespace MCD {

AudioBuffer::AudioBuffer(const Path& fileId, size_t bufferCount)
	: Resource(fileId), mBufferCount(bufferCount)
{
	if(mBufferCount > MCD_COUNTOF(handles))
		mBufferCount = MCD_COUNTOF(handles);
	memset(handles, 0, sizeof(handles));
	alGenBuffers(mBufferCount, handles);
}

AudioBuffer::~AudioBuffer()
{
	alDeleteBuffers(mBufferCount, handles);
}

}	// namespace MCD
