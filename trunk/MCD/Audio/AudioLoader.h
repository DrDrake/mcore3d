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

	virtual void abortLoad() = 0;

	/*!	Ask the loader about any newly loaded buffer.
		\return The index of which sub-buffer in AudioBuffer is loaded, -1 if nothing new is loaded.
	 */
	virtual int popLoadedBuffer() = 0;

	struct Info
	{
		size_t frequency;
		uint64_t totalPcm;	//!< Total length of the audio in unit of PCM.
	};	// Info

	virtual Info info() const = 0;

	/*!	The current read offset in the audio PCM stream.
		Since the load operation may performed in multi-thread environment, the more accurate
		definition of the offset should be how many PCM from the beginning is already passed
		by the repeatitive call of popLoadedBuffer().
	 */
	virtual uint64_t pcmOffset() const = 0;

	//!	Seek to a specific pcm offset position, if the binded IO stream supports seeking.
	virtual sal_checkreturn bool seek(uint64_t pcmOffset) = 0;
};	// IAudioStreamLoader

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOLOADER__
