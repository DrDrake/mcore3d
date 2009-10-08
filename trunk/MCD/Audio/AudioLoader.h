#ifndef __MCD_AUDIO_AUDIOLOADER__
#define __MCD_AUDIO_AUDIOLOADER__

#include "../Core/System/IntrusivePtr.h"

namespace MCD {

typedef IntrusivePtr<class AudioBuffer> AudioBufferPtr;

class MCD_ABSTRACT_CLASS IAudioStreamLoader
{
protected:
	virtual ~IAudioStreamLoader() {}

public:
	/*!	Invoked by AudioSource when new buffer data need to be load.
		Each request will be queued up and executed inside the commit() function after the load is finished.
		Since AudioBuffer has several internal buffers, a bufferIndex need to be supplied inorder to
		identify which buffer to load.
	 */
	virtual void requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex) = 0;
};	// IAudioStreamLoader

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOLOADER__
