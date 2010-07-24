#ifndef __MCD_AUDIO_AUDIOBUFFER__
#define __MCD_AUDIO_AUDIOBUFFER__

#include "ShareLib.h"
#include "../Core/System/Resource.h"

namespace MCD {

// TODO: Temp
bool MCD_AUDIO_API checkAndPrintError(const char* prefixMessage);

/*!	A very light weighted class storing an array of handle to the underlaying audio API.
	To retreive more information about the sub buffers (eg. frequency, channels) use
	the underlaying audio API.
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

	/*!	Get the PCM for the specific sub buffer's handle.
		Returns 0 if there is error.

		\note PCM = sizeInBytes / (channels * bitDepth / bitsPerByte)
	 */
	static size_t getPcm(uint handle);

protected:
	sal_override ~AudioBuffer();

	size_t mBufferCount;
};	// AudioBuffer

typedef IntrusivePtr<AudioBuffer> AudioBufferPtr;

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOBUFFER__
