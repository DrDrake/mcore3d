#ifndef __MCD_AUDIO_AUDIOLOADER__
#define __MCD_AUDIO_AUDIOLOADER__

#include "../Core/System/IntrusivePtr.h"

namespace MCD {

typedef IntrusivePtr<class AudioBuffer> AudioBufferPtr;

/*!	An interface designed to aid an IResourceLoader to continue partial load on demand.
	The interface functions are suppose to be used by AudioSource, which has knowledge
	about which audio buffer is processed and when to load next.
 */
class MCD_ABSTRACT_CLASS IAudioStreamLoader
{
protected:
	virtual ~IAudioStreamLoader() {}

public:
	/*!	Invoked by AudioSource when new buffer data need to be load.
		Each request will be queued up and executed after the load is finished.
		Since AudioBuffer has several internal buffers, a bufferIndex need to be supplied inorder to
		identify which buffer to load.
	 */
	virtual void requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex) = 0;

	/*!	Ask the loader about any newly loaded buffer.
		\return The index of which sub-buffer in AudioBuffer is loaded, -1 if nothing new is loaded.
	 */
	virtual int popLoadedBuffer() = 0;
};	// IAudioStreamLoader

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOLOADER__
