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
}

AudioSource::~AudioSource()
{
	alDeleteSources(1, &handle);
}

bool AudioSource::load(IResourceManager& resourceManager, const Path& fileId)
{
	IResourceLoaderPtr _loader = nullptr;
	ResourcePtr res = resourceManager.load(fileId, false, 0, nullptr, &_loader);
	loader = dynamic_cast<IAudioStreamLoader*>(_loader.get()) ? _loader : nullptr;
	buffer = dynamic_cast<AudioBuffer*>(res.get());

	// TODO: Log
	if(!loader || !buffer)
		return false;

	return true;
}

void AudioSource::play()
{
	if(!buffer)
		return;

	// Do actual play
	alSourcePlay(handle);
}

void AudioSource::stop()
{
	if(!buffer)
		return;

	// Stop the Source and clear the Queue
	alSourceStop(handle);
	alSourcei(handle, AL_BUFFER, 0);
}

void AudioSource::update()
{
	if(!buffer)
		return;

	// Check which buffer is played

	// Tells resource Loader continue to load
	// loader.continueLoad();

	// Request the number of OpenAL Buffers have been processed (played) on the Source
	ALint iBuffersProcessed = 0;
	alGetSourcei(handle, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

	// For each processed buffer, remove it from the Source Queue, read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the Source Queue
	while(iBuffersProcessed)
	{
		// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
		ALuint uiBuffer = 0;
		alSourceUnqueueBuffers(handle, 1, &uiBuffer);

		// Read more audio data (if there is any)
/*		ulBytesWritten = DecodeOggVorbis(&sOggVorbisFile, pDecodeBuffer, ulBufferSize, ulChannels);
		if (ulBytesWritten)
		{
			alBufferData(uiBuffer, ulFormat, pDecodeBuffer, ulBytesWritten, ulFrequency);
			alSourceQueueBuffers(handle, 1, &uiBuffer);
		}*/

		--iBuffersProcessed;
	}
}

}	// namespace MCD
