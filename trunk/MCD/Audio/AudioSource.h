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
	sal_checkreturn bool load(IResourceManager& resourceManager, const Path& fileId, sal_in_z_opt const wchar_t* args=nullptr);

	void play();

	void stop();

	//! Check buffer status...
	void update();

// Attributes
	uint handle;

	IResourceLoaderPtr loader;

	AudioBufferPtr buffer;

	/*!	Returns whether the audio source is playing.
		This function defins the playing status at the very low level; if the source is temporary
		out of buffer, this function will return false until the buffer gets loaded again.
	 */
	bool isPlaying() const;

private:
	bool mRequestPlay;
};	// AudioSource

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOSOURCE__
