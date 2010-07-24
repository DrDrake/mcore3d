#ifndef __MCD_BINDING_AUDIOCOMPONENT__
#define __MCD_BINDING_AUDIOCOMPONENT__

#include "ShareLib.h"
#include "../Component/Audio/AudioEffectComponent.h"
#include "../Component/Audio/AudioSourceComponent.h"
#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::AudioComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::AudioEffectComponent, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::AudioSourceComponent, MCD_BINDING_API);

}	// namespace script

#endif	// __MCD_BINDING_AUDIOCOMPONENT__
