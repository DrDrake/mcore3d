#include "Pch.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "AudioLoader.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"
#include "../../3Party/OpenAL/al.h"

namespace MCD {

AudioSource::AudioSource()
{
	alGenSources(1, &handle);
	mRequestPlay = false;
}

AudioSource::~AudioSource()
{
	alDeleteSources(1, &handle);
}

class FakeCallback : public ResourceManagerCallback
{
public:
	sal_override void doCallback()
	{
		++count;
	}

	static size_t count;
};	// FakeCallback

bool AudioSource::load(IResourceManager& resourceManager, const Path& fileId, const wchar_t* args)
{
	IAudioStreamLoader* _loader = nullptr;
	ResourcePtr res = resourceManager.load(fileId, false, 0, args, &loader);
	_loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	buffer = dynamic_cast<AudioBuffer*>(res.get());

	// TODO: Log
	if(!_loader || !buffer)
		return false;

	// Each streamming audio should have it's own buffer, so we uncache it from the manager.
	resourceManager.uncache(fileId);

	// Fill up the initial buffers
	for(size_t i=0; i<buffer->bufferCount(); ++i)
		_loader->requestLoad(buffer, i);

	return true;
}

void AudioSource::play()
{
	mRequestPlay = true;
}

void AudioSource::stop()
{
	mRequestPlay = false;
}

void AudioSource::update()
{
	if(!buffer)
		return;

	IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get());

	// Request the number of OpenAL Buffers have been processed (played) on the Source
	ALint buffersProcessed = 0;
	alGetSourcei(handle, AL_BUFFERS_PROCESSED, &buffersProcessed);

	// For each processed buffer, remove it from the Source Queue
	while(buffersProcessed)
	{
		// Remove the Buffer from the Queue. (uiBuffer contains the Buffer ID for the unqueued Buffer)
		ALuint uiBuffer = 0;
		alSourceUnqueueBuffers(handle, 1, &uiBuffer);

		// Tells resource loader continue to load
		for(size_t i=0; i<buffer->bufferCount(); ++i) {
			if(buffer->handles[i] == uiBuffer) {
				_loader->requestLoad(buffer, i);
				break;
			}
		}

		--buffersProcessed;
	}

	// Queue new buffers that were loaded by the loader
	int bufferIdx;
	while((bufferIdx = _loader->popLoadedBuffer()) != -1) {
		alSourceQueueBuffers(handle, 1, &buffer->handles[bufferIdx]);
		checkAndPrintError("alSourceQueueBuffers failed: ");
	}

	if(!isPlaying() && mRequestPlay) {
		// Do actual play
		alSourcePlay(handle);
		checkAndPrintError("alSourcePlay failed: ");
	}
	else if(isPlaying() && !mRequestPlay) {
		// Stop the Source and clear the Queue
		alSourceStop(handle);
		alSourcei(handle, AL_BUFFER, 0);
	}
}

bool AudioSource::isPlaying() const
{
	ALint val;
	alGetSourcei(handle, AL_SOURCE_STATE, &val);
	return val == AL_PLAYING;
}

}	// namespace MCD
