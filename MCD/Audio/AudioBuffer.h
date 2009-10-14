#ifndef __MCD_AUDIO_AUDIOBUFFER__
#define __MCD_AUDIO_AUDIOBUFFER__

#include "ShareLib.h"
#include "../Core/System/Resource.h"

namespace MCD {

// TODO: Move to a better place
void MCD_AUDIO_API initAudio();

// TODO: Temp
bool MCD_AUDIO_API checkAndPrintError(const char* prefixMessage);

/*!	A very light weighted class storing an array of
 */
class MCD_AUDIO_API AudioBuffer : public Resource
{
public:
	explicit AudioBuffer(const Path& fileId, size_t bufferCount);

// Operations

// Attributes
	static const size_t cMaxBuffers = 8;

	// TODO: Make the size dynamic?
	uint handles[cMaxBuffers];

	size_t bufferCount() const {
		return mBufferCount;
	}

protected:
	sal_override ~AudioBuffer();

	size_t mBufferCount;
};	// AudioBuffer

typedef IntrusivePtr<AudioBuffer> AudioBufferPtr;

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOBUFFER__
