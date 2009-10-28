#ifndef __MCD_AUDIO_AUDIOEFFECT__
#define __MCD_AUDIO_AUDIOEFFECT__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

// TODO: Move to a better place
void MCD_AUDIO_API initAudioEffect();

class AudioSource;

/*!
 */
class MCD_AUDIO_API AudioEffect : Noncopyable
{
public:
	AudioEffect();

	~AudioEffect();

// Operations
	void create(sal_in_z_opt const char* effectName=nullptr);

	void bind(AudioSource& source);

	void setParameters(sal_in_z_opt const char* paramNameValuePair=nullptr);

	float getParameterf(sal_in_z_opt const char* paramName=nullptr);

// Attributes
	uint handle;
	uint slotHandle;
};	// AudioBuffer

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOEFFECT__
