#ifndef __MCD_AUDIO_ALINCLUDE__
#define __MCD_AUDIO_ALINCLUDE__

#include "../Core/System/Platform.h"

#ifdef MCD_APPLE
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#else
#	include "../../3Party/OpenAL/al.h"
#	include "../../3Party/OpenAL/alc.h"
#endif

#endif	// __MCD_AUDIO_ALINCLUDE__
