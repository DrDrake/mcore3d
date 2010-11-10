#ifndef __MCD_AUDIO_AUDIOBINDING__
#define __MCD_AUDIO_AUDIOBINDING__

#include "ShareLib.h"
#include "../Core/Binding/ClassTraits.h"

namespace MCD {
namespace Binding {

MCD_AUDIO_API void registerAudioBinding(VMCore& vm);

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOBINDING__
