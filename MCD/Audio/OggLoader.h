#ifndef __MCD_AUDIO_OGGLOADER__
#define __MCD_AUDIO_OGGLOADER__

#include "ShareLib.h"
#include "AudioLoader.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

/*!
	Different to others loader, streaming audio loader can only commit the data
	to a single resource (AudioBuffer) only.
 */
class MCD_AUDIO_API OggLoader : public IResourceLoader, public IAudioStreamLoader, private Noncopyable
{
public:
	OggLoader();

	sal_override ~OggLoader();

// Operations
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr);

	sal_override void commit(Resource& resource);

	/*!	Invoked by AudioSource when new buffer data need to be load.
		Each request will be queued up and executed inside the commit() function after the load is finished.
		Since AudioBuffer has several internal buffers, a bufferIndex need to be supplied inorder to
		identify which buffer to load.
	 */
	sal_override void requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex);

	sal_override int popLoadedBuffer();

// Attributes
	sal_override Info info() const;

	sal_override uint64_t pcmOffset() const;

	sal_override sal_checkreturn bool seek(uint64_t pcmOffset);

protected:
	class Impl;
	Impl& mImpl;
};	// OggLoader

class MCD_AUDIO_API OggLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args);
	sal_override IResourceLoaderPtr createLoader();
};	// OggLoaderFactory

}	// namespace MCD

#endif	// __MCD_AUDIO_OGGLOADER__
