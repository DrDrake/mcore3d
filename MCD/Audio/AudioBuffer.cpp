#include "Pch.h"
#include "AudioBuffer.h"
#include "../Core/System/Utility.h"
#include "../../3Party/OpenAL/al.h"

namespace MCD {

const char* getALErrorString(ALenum err)
{
	switch(err)
	{
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
	}
	return "";
}

bool checkAndPrintError(const char* prefixMessage)
{
	ALenum err = alGetError();
	if(err == AL_NO_ERROR)
		return true;

	std::cout << prefixMessage << getALErrorString(err) << std::endl;
	return false;
}

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

size_t AudioBuffer::getPcm(uint handle)
{
	if(!alIsBuffer(handle))
		return 0;

	static const size_t bitsPerByte = 8;
	ALint bits, channels, sizeInBytes;

	alGetBufferi(handle, AL_BITS, &bits);
	alGetBufferi(handle, AL_CHANNELS, &channels);
	alGetBufferi(handle, AL_SIZE, &sizeInBytes);

	if(channels * bits == 0)
		return 0;

	size_t pcm = sizeInBytes / (channels * bits / bitsPerByte);
	return pcm;
}

}	// namespace MCD
