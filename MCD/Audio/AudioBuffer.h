#ifndef __MCD_AUDIO_AUDIOBUFFER__
#define __MCD_AUDIO_AUDIOBUFFER__

#include "ShareLib.h"
#include "../Core/System/Resource.h"

namespace MCD {

class MCD_AUDIO_API AudioBuffer : public Resource
{
public:
	explicit AudioBuffer(const Path& fileId);

// Operations

// Attributes
	uint handle;

protected:
	sal_override ~AudioBuffer();
};	// AudioBuffer

typedef IntrusivePtr<AudioBuffer> AudioBufferPtr;

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOBUFFER__
