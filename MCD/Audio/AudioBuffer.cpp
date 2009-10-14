#include "Pch.h"
#include "AudioBuffer.h"
#include "../Core/System/Utility.h"
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/OpenAL/alc.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenAL32")
#endif

namespace MCD {

/*	Some usefull links:
	Offset
	http://www.nabble.com/AL_*_OFFSET-resolution-td14216950.html
	Calculate the current playing time:
	http://opensource.creative.com/pipermail/openal/2008-March/011015.html
	Jump to particular position:
	http://stackoverflow.com/questions/434599/openal-how-does-one-jump-to-a-particular-offset-more-than-once
 */

void MCD_AUDIO_API initAudio()
{
	static ALCdevice* device = nullptr;
	static ALCcontext* context = nullptr; 

	device = alcOpenDevice(nullptr);
	context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);

	ALboolean g_bEAX = alIsExtensionPresent("EAX2.0");
	(void)g_bEAX;
}

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

}	// namespace MCD
