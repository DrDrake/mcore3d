#ifndef __MCD_AUDIO_AUDIODEVICE__
#define __MCD_AUDIO_AUDIODEVICE__

#include "ShareLib.h"
#include "../Core/System/Platform.h"

namespace MCD {

/*!	Initialize the underlaying audio device, before other class like AudioBuffer and AudioSource
	can be used. It can be invoked multiple times where the same number of shutDownAudioDevice()
	have to be called for shutting down the device.

	The design of the interface assumes a single audio device, which should be sufficient for
	most gamming application.

	\return False if the audio device fail to initialize
 */
sal_checkreturn bool MCD_AUDIO_API initAudioDevice();

/*!	Shutdown the audio device.
 */
void MCD_AUDIO_API closeAudioDevice();

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIODEVICE__
