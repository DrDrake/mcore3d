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
	 */
	sal_checkreturn bool load(IResourceManager& resourceManager, const Path& fileId, bool firstBufferBlock=false, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Start to play the audio, if some of the background loading buffer becomes available.
		If currently there is no buffer to play, keep calling update() and resource manager's update().
		The real playing status can be query using isReallyPlaying().

		If the audio source is in a pause state, calling play() can resume it; if it's already playing,
		calling play() again will rewind the play position to the beginning.
	 */
	void play();

	void pause();

	void stop();

	/*!	Because of the asyn nature of audio streamming, we need an update function to do necessary
		processing.
	 */
	void update();

// Attributes
	uint handle;

	IResourceLoaderPtr loader;

	AudioBufferPtr buffer;

	bool isPlaying() const;

	/*!	Returns whether the audio source is really playing in the driver level.
		This function defins the playing status at the very low level; if the source is temporary
		out of buffer, this function will return false until the buffer gets loaded again.
	 */
	bool isReallyPlaying() const;

	bool isPaused() const;

private:
	bool mRequestPlay;
	bool mRequestPause;
};	// AudioSource

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOSOURCE__
