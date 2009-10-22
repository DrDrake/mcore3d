#include "Pch.h"
#include "OggLoader.h"
#include "AudioBuffer.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PlatformInclude.h"
#include "../Core/System/ResourceManager.h"
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/VorbisOgg/vorbisfile.h"

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <deque>
#ifdef MCD_VC
#	pragma warning(pop)
#endif

namespace MCD {

namespace {

// Ogg function pointers
typedef int (*LPOVCLEAR)(OggVorbis_File* vf);
typedef long (*LPOVREAD)(OggVorbis_File* vf,char* buffer, int length, int bigendianp, int word, int sgned, int* bitstream);
typedef ogg_int64_t (*LPOVPCMTOTAL)(OggVorbis_File* vf, int i);
typedef vorbis_info* (*LPOVINFO)(OggVorbis_File* vf, int link);
typedef vorbis_comment* (*LPOVCOMMENT)(OggVorbis_File* vf, int link);
typedef int (*LPOVOPENCALLBACKS)(void* datasource, OggVorbis_File* vf, char* initial, long ibytes, ov_callbacks callbacks);

LPOVCLEAR			gFnOvClear = nullptr;
LPOVREAD			gFnOvRead = nullptr;
LPOVPCMTOTAL		gFnOvPcmTotal = nullptr;
LPOVINFO			gFnOvInfo = nullptr;
LPOVCOMMENT			gFnOvComment = nullptr;
LPOVOPENCALLBACKS	gFnOvOpenCallbacks = nullptr;

bool gVorbisInited = false;

void initVorbis()
{
	if(gVorbisInited)
		return;

	// Try and load Vorbis DLLs (VorbisFile.dll will load ogg.dll and vorbis.dll)
	if(HMODULE h = ::LoadLibraryA("vorbisfile.dll"))
	{
		gFnOvClear = (LPOVCLEAR)GetProcAddress(h, "ov_clear");
		gFnOvRead = (LPOVREAD)GetProcAddress(h, "ov_read");
		gFnOvPcmTotal = (LPOVPCMTOTAL)GetProcAddress(h, "ov_pcm_total");
		gFnOvInfo = (LPOVINFO)GetProcAddress(h, "ov_info");
		gFnOvComment = (LPOVCOMMENT)GetProcAddress(h, "ov_comment");
		gFnOvOpenCallbacks = (LPOVOPENCALLBACKS)GetProcAddress(h, "ov_open_callbacks");

		if (gFnOvClear && gFnOvRead && gFnOvPcmTotal && gFnOvInfo &&
			gFnOvComment && gFnOvOpenCallbacks)
		{
			gVorbisInited = true;
		}
	}
}

// Wrap around std::iostream into Ogg's callback
size_t ov_read_func(void* ptr, size_t eleSize, size_t count, void* userData)
{
	std::istream* is = reinterpret_cast<std::istream*>(userData);
	// NOTE: is->read() + is->gcount() didn't work as expected.
	// use the underlaying buffer is more reliable.
	return is->rdbuf()->sgetn((char*)ptr, eleSize * count);
}

// Return 0 on success >0 on fail, -1 if non seekable
int ov_seek_func(void* userData, ogg_int64_t offset, int whence)
{
	std::istream* is = reinterpret_cast<std::istream*>(userData);
	if(whence == SEEK_SET)
		is->seekg(long(offset), std::ios_base::beg);
	else if(whence == SEEK_CUR)
		is->seekg(long(offset), std::ios_base::cur);
	else if(whence == SEEK_END)
		is->seekg(long(offset), std::ios_base::end);
	else
		return -1;

	return (is->eof() || is->bad()) ? 1 : 0;
}

int ov_close_func(void* userData)
{
	return 0;
}

long ov_tell_func(void* userData)
{
	std::istream* is = reinterpret_cast<std::istream*>(userData);
	return is->tellg();
}

void swap(short& s1, short& s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

size_t gDecodeOggVorbis(OggVorbis_File* psOggVorbisFile, char* pDecodeBuffer, size_t ulBufferSize, size_t ulChannels)
{
	int current_section;
	long lDecodeSize;
	unsigned long ulSamples;
	short* pSamples;

	unsigned long ulBytesDone = 0;
	while (true)
	{
		lDecodeSize = gFnOvRead(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;

			if (ulBytesDone >= ulBufferSize)
				break;
		}
		else
		{
			break;
		}
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (ulChannels == 6)
	{		
		pSamples = (short*)pDecodeBuffer;
		for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
			swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}

	return ulBytesDone;
}

// Calculate how large a single buffer is needed in order to play a sound with duration in milli seconds.
size_t calculateBufferSize(size_t bufferDurationMs, vorbis_info* info)
{
	size_t bufferInByte = 0;
	size_t channel2 = info->channels * 2;

	// Frequency * 2 (16bit) * fraction of a second
	bufferInByte = info->rate * channel2 * bufferDurationMs / 1000;
	// Important: The buffer Size must be an exact multiple of the block alignment.
	bufferInByte -= (bufferInByte % channel2);

	return bufferInByte;
}

ALenum getFormat(vorbis_info* info)
{
	switch(info->channels) {
	case 1:
		return AL_FORMAT_MONO16;
	case 2:
		return AL_FORMAT_STEREO16;
	case 4:
		return alGetEnumValue("AL_FORMAT_QUAD16");
	case 6:
		return alGetEnumValue("AL_FORMAT_51CHN16");
	default:
		MCD_ASSERT(false);
		return -1;
	}
}

}	// namespace

class OggLoader::Impl
{
public:
	struct Task
	{
		int bufferIdx;
		AudioBufferPtr buffer;	// TODO: May not needed
	};	// Task

	class TaskQueue
	{
	public:
		void push(const Task& task)
		{
			ScopeLock lock(mMutex);
			mQueue.push_back(task);
		}

		Task pop()
		{
			ScopeLock lock(mMutex);
			
			Task ret = { -1, nullptr };
			if(!mQueue.empty()) {
				ret = mQueue.front();
				mQueue.pop_front();
			}
			return ret;
		}

		bool isEmpty() const
		{
			ScopeLock lock(mMutex);
			return mQueue.empty();
		}

		std::deque<Task> mQueue;
		mutable Mutex mMutex;
	};	// TaskQueue

	Impl()
		: partialLoadContext(nullptr), resourceManager(nullptr)
		, mHeaderLoaded(false), mIStream(nullptr)
	{}

	~Impl()
	{
		gFnOvClear(&mOggFile);
	}

	bool loadHeader(std::istream* is)
	{
		MCD_ASSERT(mMutex.isLocked());

		if(mHeaderLoaded)
			return true;

		mIStream = is;
		mOggFileCallbacks.read_func = ov_read_func;
		mOggFileCallbacks.seek_func = ov_seek_func;
		mOggFileCallbacks.close_func = ov_close_func;
		mOggFileCallbacks.tell_func = ov_tell_func;

		{	ScopeUnlock unlock(mMutex);
			if(gFnOvOpenCallbacks(is, &mOggFile, nullptr, 0, mOggFileCallbacks) != 0)
				return false;

			// Get some information about the file (Channels, Format, and Frequency)
			mVorbisInfo = gFnOvInfo(&mOggFile, -1);
			if(!mVorbisInfo)
				return false;

			mBufferSize = calculateBufferSize(250, mVorbisInfo);
			format = getFormat(mVorbisInfo);
			frequency = mVorbisInfo->rate;
		}

		mHeaderLoaded = true;
		return mHeaderLoaded;
	}

	//! Returns -1 for error, 0 for eof, 1 for success
	int loadData()
	{
		MCD_ASSERT(mMutex.isLocked());

		if(!mIStream)
			return -1;

		// Try to process all the items in the task queue
		while(true)
		{
			Task task = mTaskQueue.pop();
			if(task.bufferIdx == -1 || !task.buffer)
				break;

			ScopeUnlock unlock(mMutex);

			// TODO: Optimize to use less dynamic allocation
			void* buf = ::malloc(mBufferSize);
			if(!buf)
				return -1;

			AudioBuffer& buffer = *task.buffer;
			size_t bytesWritten = gDecodeOggVorbis(&mOggFile, (char*)buf, mBufferSize, mVorbisInfo->channels);
			// NOTE: Make sure no other thread is using this buffer handle, otherwise we
			// need to move the alBufferData() function to do inside commit().
			alBufferData(buffer.handles[task.bufferIdx], format, buf, bytesWritten, frequency);

			Task task2 = { task.bufferIdx, nullptr };
			mCommitQueue.push(task2);

			if(!checkAndPrintError("alBufferData failed: "))
				return -1;

			::free(buf);

			// Eof
			if(bytesWritten < mBufferSize)
				return 0;
		}

		return 1;
	}

	void requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex)
	{
		Task taks = { int(bufferIndex), buffer };
		mTaskQueue.push(taks);

		ScopeLock lock(mMutex);

		// If partialLoadContext is null, a loading is already in progress,
		// onPartialLoaded() will be invoked soon.
		if(!resourceManager || !partialLoadContext)
			return;

		// Otherwise we can tell resource manager to re-schedule the load immediatly.
		resourceManager->reSchedule(partialLoadContext, 0, nullptr);
		resourceManager = nullptr;
		partialLoadContext = nullptr;
	}

	int popLoadedBuffer()
	{
		Task task = mCommitQueue.pop();
		return task.bufferIdx;
	}

	size_t mBufferSize;	//!< Calculated suitable buffer size for each buffer in AudioBuffer.
	ALenum format;
	ALsizei frequency;
	void* partialLoadContext;
	IResourceManager* resourceManager;
	Mutex mMutex;

	TaskQueue mTaskQueue, mCommitQueue;

	// Information about the Ogg file
	bool mHeaderLoaded;
	OggVorbis_File mOggFile;
	ov_callbacks mOggFileCallbacks;
	vorbis_info* mVorbisInfo;
	std::istream* mIStream;
};	// Impl

OggLoader::OggLoader()
{
	initVorbis();
	mImpl = new Impl();
}

OggLoader::~OggLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState OggLoader::load(std::istream* is, const Path*, const wchar_t*)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	if(!is)
		return loadingState = Aborted;
	else if(loadingState == Aborted)
		loadingState = NotLoaded;

	if(!mImpl->loadHeader(is))
		return loadingState = Aborted;

	int result = mImpl->loadData();
	if(result == 1)
		loadingState = PartialLoaded;
	else if(result == 0)
		loadingState = Loaded;
	else
		loadingState = Aborted;

	return loadingState;
}

void OggLoader::commit(Resource& resource)
{
	// Currently nothing to do inside commit
}

IResourceLoader::LoadingState OggLoader::getLoadingState() const
{
	// We don't have arithmetics on loadingState, so we don't need to lock on it
	return loadingState;
}

// Invoked in resource manager worker thread
void OggLoader::onPartialLoaded(IResourceManager& manager, void* context, uint priority, const wchar_t* args)
{
	MCD_ASSUME(mImpl);

	if(mImpl->mTaskQueue.isEmpty()) {
		ScopeLock lock(mImpl->mMutex);
		mImpl->partialLoadContext = context;
		mImpl->resourceManager = &manager;
	} else {
		// Re-schedule immediatly if the task queue isn't empty,
		// this situation is rare but possible when the load() function completes in
		// it's worker thread and then someone call requestLoad() in main thread
		// while onPartialLoaded() is not yet invoked.
		manager.reSchedule(context, priority, args);
	}
}

// Invoked by user or AudioSource
void OggLoader::requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex)
{
	mImpl->requestLoad(buffer, bufferIndex);
}

int OggLoader::popLoadedBuffer()
{
	return mImpl->popLoadedBuffer();
}

}	// namespace MCD
