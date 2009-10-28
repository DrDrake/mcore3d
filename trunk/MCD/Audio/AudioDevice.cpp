#include "Pch.h"
#include "AudioDevice.h"
#include "../Core/System/Log.h"
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/OpenAL/alc.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenAL32")
#endif

namespace MCD {

static ALCdevice* gDevice = nullptr;
static ALCcontext* gContext = nullptr; 
static size_t gInitCount = 0;

/*	Some usefull links about OpenAL:
	Offset
	http://www.nabble.com/AL_*_OFFSET-resolution-td14216950.html
	Calculate the current playing time:
	http://opensource.creative.com/pipermail/openal/2008-March/011015.html
	Jump to particular position:
	http://stackoverflow.com/questions/434599/openal-how-does-one-jump-to-a-particular-offset-more-than-once
 */
bool initAudioDevice()
{
	if(gInitCount > 0)
		return true;

	gDevice = alcOpenDevice(nullptr);

	if(!gDevice) {
		Log::write(Log::Error, L"Fail to initialize audio device");
		return false;
	}

	gContext = alcCreateContext(gDevice, nullptr);
	alcMakeContextCurrent(gContext);

	ALboolean hasEax = alIsExtensionPresent("EAX2.0");
	(void)hasEax;

	++gInitCount;
	return true;
}

void closeAudioDevice()
{
	if(--gInitCount)
		return;

	ALboolean ok;
	ok = alcMakeContextCurrent(nullptr);
	alcDestroyContext(gContext);
	ok = alcCloseDevice(gDevice);
	(void)ok;
}

}	// namespace MCD
