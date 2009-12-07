#include "Pch.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "AudioLoader.h"
#include "../Core/System/Log.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"	// for NvpParser
#include "../Core/System/Thread.h"	// for mSleep()
#include "../../3Party/OpenAL/al.h"

namespace MCD {

AudioSource::AudioSource()
{
	alGenSources(1, &handle);
	checkAndPrintError("alGenSources failed: ");
	mRequestPlay = false;
	mRequestPause = false;
	mRoughPcmOffsetSinceLastSeek = 0;
}

AudioSource::~AudioSource()
{
	alDeleteSources(1, &handle);

	// Cancel any pended loading operations
	IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	if(_loader)
		_loader->abortLoad();
}

bool AudioSource::load(IResourceManager& resourceManager, const Path& fileId, const wchar_t* args)
{
	// Determine the "blockLoadFirstBuffer" option in args
	bool blockLoadFirstBuffer = false;
	if(args)
	{
		NvpParser parser(args);
		const wchar_t* name, *value;
		while(parser.next(name, value))
		{
			if(wstrCaseCmp(name, L"blockLoadFirstBuffer") == 0) {
				blockLoadFirstBuffer = (wStr2IntWithDefault(value, 0) > 0);
				break;
			}
		}
	}

	IAudioStreamLoader* _loader = nullptr;
	ResourcePtr res = resourceManager.load(
		fileId, blockLoadFirstBuffer ? IResourceManager::FirstPartialBlock : IResourceManager::NonBlock,
		0, args, &loader
	);

	_loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	buffer = dynamic_cast<AudioBuffer*>(res.get());

	if(!_loader || !buffer) {
		Log::format(Log::Error, L"Fail to load audio: %s", fileId.getString().c_str());
		return false;
	}

	// Each streamming audio should have it's own buffer, so we uncache it from the manager.
	resourceManager.uncache(fileId);

	fillUpInitialBuffers();

	if(blockLoadFirstBuffer) {
		int bufferIdx;

		// Wait until the worker thread give us response.
		while((bufferIdx = _loader->popLoadedBuffer()) == -1)
			MCD::mSleep(1);

		alSourceQueueBuffers(handle, 1, &buffer->handles[bufferIdx]);
		checkAndPrintError("alSourceQueueBuffers failed: ");
	}

	return true;
}

void AudioSource::play()
{
	// TODO: Implement the rewind feature for streamming source
	alSourcePlay(handle);
	mRequestPlay = true;
	mRequestPause = false;
}

void AudioSource::pause()
{
	alSourcePause(handle);
	mRequestPause = true;
}

void AudioSource::stop()
{
	stopAndUnqueueBuffers();
	mRequestPlay = false;
}

bool AudioSource::seek(uint64_t pcmOffset)
{
	IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	if(!_loader)
		return false;

	if(!_loader->seek(pcmOffset))
		return false;

	stopAndUnqueueBuffers();

	// Discard all remaining loaded buffer in the loader
	while(_loader->popLoadedBuffer() != -1) {}

	fillUpInitialBuffers();

	mRoughPcmOffsetSinceLastSeek = pcmOffset;

	return true;
}

void AudioSource::update()
{
	if(!buffer || mRequestPause)
		return;

	if(IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get()))
	{
		// Request the number of OpenAL Buffers have been processed (played) on the Source
		ALint buffersProcessed = 0;
		alGetSourcei(handle, AL_BUFFERS_PROCESSED, &buffersProcessed);

		// For each processed buffer, remove it from the Source Queue
		while(buffersProcessed)
		{
			// Remove the Buffer from the Queue. (uiBuffer contains the Buffer ID for the unqueued Buffer)
			ALuint uiBuffer = 0;
			alSourceUnqueueBuffers(handle, 1, &uiBuffer);
			checkAndPrintError("alSourceUnqueueBuffers failed: ");

			mRoughPcmOffsetSinceLastSeek += AudioBuffer::getPcm(uiBuffer);

			// Tells resource loader continue to load, if the audio is not ended.
			if(!isPcmPlayToEnd()) for(size_t i=0; i<buffer->bufferCount(); ++i) {
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
	}

	if(isPcmPlayToEnd())
		stop();

	bool reallyPlaying = isReallyPlaying();

	if(!reallyPlaying && mRequestPlay) {
		// Do actual play
		alSourcePlay(handle);
		checkAndPrintError("alSourcePlay failed: ");
	}
	else if(reallyPlaying && !mRequestPlay) {
		// Stop the Source and clear the Queue
		alSourceStop(handle);
		alSourcei(handle, AL_BUFFER, 0);
	}

	if(reallyPlaying && mRequestPause)
		alSourcePause(handle);
}

size_t AudioSource::frequency() const
{
	if(IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get()))
		return _loader->info().frequency;
	return 0;
}

uint64_t AudioSource::totalPcm() const
{
	if(IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get()))
		return _loader->info().totalPcm;
	return 0;
}

uint64_t AudioSource::currentPcm() const
{
	ALint fineOffset;
	alGetSourcei(handle, AL_SAMPLE_OFFSET, &fineOffset);

	return fineOffset + mRoughPcmOffsetSinceLastSeek;
}

bool AudioSource::isPlaying() const
{
	return mRequestPlay;
}

bool AudioSource::isReallyPlaying() const
{
	ALint val;
	alGetSourcei(handle, AL_SOURCE_STATE, &val);
	return val == AL_PLAYING;
}

bool AudioSource::isPaused() const
{
	return mRequestPause;
}

float AudioSource::gain() const
{
	ALfloat value = 0;
	alGetSourcef(handle, AL_GAIN, &value);
	return value;
}

void AudioSource::setGain(float value)
{
	alSourcef(handle, AL_GAIN, value);
}

void AudioSource::fillUpInitialBuffers()
{
	IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	if(!buffer || !_loader)
		return;

	for(size_t i=0; i<buffer->bufferCount(); ++i)
		_loader->requestLoad(buffer, i);
}

// In OpenAL, we need to manually unqueue the buffers after alSourceStop()
void AudioSource::stopAndUnqueueBuffers()
{
	alSourceStop(handle);

	ALint buffersProcessed = 0;
	alGetSourcei(handle, AL_BUFFERS_PROCESSED, &buffersProcessed);

	ALuint dummy[AudioBuffer::cMaxBuffers];
	alSourceUnqueueBuffers(handle, buffersProcessed, dummy);
	checkAndPrintError("alSourceUnqueueBuffers failed: ");
}

bool AudioSource::isPcmPlayToEnd() const
{
	MCD_ASSERT(totalPcm() == 0 || currentPcm() <= totalPcm());
	return currentPcm() == totalPcm() && totalPcm() > 0;
}

}	// namespace MCD
