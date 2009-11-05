#ifndef __MCD_COMPONENT_AUDIO_AUDIOEFFECTCOMPONENT__
#define __MCD_COMPONENT_AUDIO_AUDIOEFFECTCOMPONENT__

#include "AudioSourceComponent.h"
#include "../../Audio/AudioEffect.h"

namespace MCD {

/*!	
 */
class MCD_COMPONENT_API AudioEffectComponent : public AudioComponent
{
public:
// Operations
	sal_override void update();

	void create(sal_in_z_opt const char* effectName) { audioEffect.create(effectName); }

	AudioEffect audioEffect;
};	// AudioEffectComponent

typedef WeakPtr<AudioEffectComponent> AudioEffectComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_AUDIO_AUDIOEFFECTCOMPONENT__
