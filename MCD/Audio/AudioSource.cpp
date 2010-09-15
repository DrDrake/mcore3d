#include "Pch.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "AudioLoader.h"
#include "ALInclude.h"
#include "../Core/System/Log.h"
#include "../Core/System/ResourceLoader.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"	// for NvpParser
#include "../Core/System/Thread.h"	// for mSleep()

namespace MCD {

AudioSource::AudioSource()
{
	::alGetError();
	::alGenSources(1, &handle);
	checkAndPrintError("alGenSources failed: ");
	mRequestPlay = false;
	mRequestPause = false;
	mRoughPcmOffsetSinceLastSeek = 0;
	mResourceManager = nullptr;
}

AudioSource::~AudioSource()
{
	alDeleteSources(1, &handle);
}

bool AudioSource::load(ResourceManager& resourceManager, const Path& fileId, const char* args)
{
	// Determine the "blockLoadFirstBuffer" option in args
	bool blockLoadFirstBuffer = false;
	if(args)
	{
		NvpParser parser(args);
		const char* name, *value;
		while(parser.next(name, value))
		{
			if(strCaseCmp(name, "blockLoadFirstBuffer") == 0) {
				blockLoadFirstBuffer = (str2IntWithDefault(value, 0) > 0);
				break;
			}
		}
	}

	ResourcePtr res = resourceManager.load(
		fileId, blockLoadFirstBuffer ? 1 : -1,
		0, args
	);

	loader = resourceManager.getLoader(fileId);
	IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get());
	buffer = dynamic_cast<AudioBuffer*>(res.get());

	if(!_loader || !buffer) {
		Log::format(Log::Error, "Fail to load audio: %s", fileId.getString().c_str());
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

	mFileId = fileId;
	mLoadOptions = args ? args : "";
	mResourceManager = &resourceManager;

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
	mRoughPcmOffsetSinceLastSeek = 0;
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
			alGetError();
			alSourceUnqueueBuffers(handle, 1, &uiBuffer);
			checkAndPrintError("alSourceUnqueueBuffers failed: ");

			uint64_t elaspedPcm = AudioBuffer::getPcm(uiBuffer);
			MCD_ASSERT(elaspedPcm > 0 && "Make sure the audio loader won't gives data of zero length");
			mRoughPcmOffsetSinceLastSeek += elaspedPcm;

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
			alGetError();
			alSourceQueueBuffers(handle, 1, &buffer->handles[bufferIdx]);
			checkAndPrintError("alSourceQueueBuffers failed: ");
		}
	}

	if(isPcmPlayToEnd())
		pause();

	const bool reallyPlaying = isReallyPlaying();

	if(!reallyPlaying && mRequestPlay) {
		// Do actual play
		alGetError();
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

size_t AudioSource::channelCount() const
{
	if(IAudioStreamLoader* _loader = dynamic_cast<IAudioStreamLoader*>(loader.get()))
		return _loader->info().channelCount;
	return 0;
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

bool AudioSource::isPcmPlayToEnd() const
{
	MCD_ASSERT(totalPcm() == 0 || currentPcm() <= totalPcm());
	const uint64_t total = totalPcm();
	return currentPcm() == total && total > 0;
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

Vec3f AudioSource::position() const
{
	Vec3f ret;
	::alGetSourcefv(handle, AL_POSITION, ret.data);
	return ret;
}

void AudioSource::setPosition(const Vec3f& p)
{
	::alSourcefv(handle, AL_POSITION, p.data);
}

Vec3f AudioSource::velocity() const
{
	Vec3f ret;
	::alGetSourcefv(handle, AL_VELOCITY, ret.data);
	return ret;
}

void AudioSource::setVelocity(const Vec3f& v)
{
	::alSourcefv(handle, AL_VELOCITY, v.data);
}

const Path& AudioSource::fileId() const
{
	return mFileId;
}

const std::string& AudioSource::loadOptions() const
{
	return mLoadOptions;
}

ResourceManager* AudioSource::resourceManager() const
{
	return mResourceManager;
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

}	// namespace MCD
