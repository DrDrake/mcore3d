#ifndef __MCD_AUDIO_AUDIOSOURCE__
#define __MCD_AUDIO_AUDIOSOURCE__

#include "ShareLib.h"
#include "../Core/System/Resource.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class IResourceLoader;
class IResourceManager;
typedef IntrusivePtr<class AudioBuffer> AudioBufferPtr;

class MCD_AUDIO_API AudioSource : Noncopyable
{
public:
	AudioSource();

	~AudioSource();

// Operations
	sal_checkreturn bool load(IResourceManager& resourceManager, const Path& fileId);

	void play();

	void stop();

	//! Check buffer status...
	void update();

// Attributes
	uint handle;

	SharedPtr<IResourceLoader> loader;

	AudioBufferPtr buffer;
};	// AudioSource

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOSOURCE__
