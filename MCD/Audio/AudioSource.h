#ifndef __MCD_AUDIO_AUDIOSOURCE__
#define __MCD_AUDIO_AUDIOSOURCE__

#include "ShareLib.h"
#include "../Core/System/Resource.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class IResourceManager;
typedef IntrusivePtr<class AudioBuffer> AudioBufferPtr;
typedef SharedPtr<class IResourceLoader> IResourceLoaderPtr;

/*!	Represent an audio source, which manage the lower level AudioBuffer and it's loader.
	\todo Make a prioritized source pooling system.
 */
class MCD_AUDIO_API AudioSource : Noncopyable
{
public:
	AudioSource();

	~AudioSource();

// Operations
	/*!	Initiate a load operation.
		This load function will create the loader and buffer member variable.
		It will fill up the buffer as much as possible, but if the audio is larger than the buffer,
		the loading will be continue on demand.
		Therefore, there is no distintion between streamming and non-streamming audio, stream or not
		is just simply depends on the buffer size.

		\param args Extra parameter as a name-value pair string that will pass to the audio loader.
			Currently supported in OggLoader:
			Name: bufferCount, Value: 1 - AudioBuffer::cMaxBuffers, Default: AudioBuffer::cMaxBuffers
			Name: subBufferLength, Value: 10 - *, Default: 250, Desc: Duration of each sub-buffer in milli seconds
			Name: blockLoadFirstBuffer, Value: 0/1, Default: 0, Desc:
				If set to true, the call will block until the first audio buffer finished loading, such that
				the audio source can play immediatly and the source's property like frequency() and totalPcm()
				can be get.
				If it's set to false, all the property funtions like frequency() and totalPcm() will not gives
				valid result until the first buffer is loaded. While the play() function will pend its operation
				until the update() function detected there is buffer loaded.
	 */
	sal_checkreturn bool load(
		IResourceManager& resourceManager, const Path& fileId,
		sal_in_z_opt const wchar_t* args=nullptr
	);

	/*!	Start to play the audio, if some of the background loading buffer becomes available.
		If currently there is no buffer to play, keep calling update() and resource manager's update().
		The real playing status can be query using isReallyPlaying().

		If the audio source is in a pause state, calling play() can resume it; if it's already playing,
		calling play() again will rewind the play position to the beginning.
	 */
	void play();

	void pause();

	void stop();

	sal_checkreturn bool seek(uint64_t pcmOffset);

	/*!	Because of the asyn nature of audio streamming, we need an update function to do necessary
		processing.
	 */
	void update();

// Attributes
	uint handle;

	IResourceLoaderPtr loader;

	AudioBufferPtr buffer;

	size_t frequency() const;

	/*!	Total length of the audio in unit of PCM.
		Note that the total time in seconds can be calculated by totalPcm() / frequency().
	 */
	uint64_t totalPcm() const;

	uint64_t currentPcm() const;

	bool isPlaying() const;

	/*!	Returns whether the audio source is really playing in the driver level.
		This function defins the playing status at the very low level; if the source is temporary
		out of buffer, this function will return false until the buffer gets loaded again.
	 */
	bool isReallyPlaying() const;

	bool isPaused() const;

	float gain() const;

	//! Set the volumn from 0 to 1, default is 1.
	void setGain(float value);

private:
	void fillUpInitialBuffers();
	void stopAndUnqueueBuffers();

	bool mRequestPlay;
	bool mRequestPause;
	uint64_t mRoughPcmOffsetSinceLastSeek;
};	// AudioSource

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOSOURCE__
