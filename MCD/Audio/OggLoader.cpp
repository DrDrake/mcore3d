#include "Pch.h"
#include "OggLoader.h"
#include "AudioBuffer.h"
#include "../Core/System/Deque.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PlatformInclude.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/VorbisOgg/vorbis/vorbisfile.h"
#include <vector>

namespace MCD {

namespace {

// Ogg function pointers
// Reference of vorbisfile: http://xiph.org/vorbis/doc/vorbisfile/overview.html
typedef int (*LPOVCLEAR)(OggVorbis_File* vf);
typedef long (*LPOVREAD)(OggVorbis_File* vf, char* buffer, int length, int bigendianp, int word, int sgned, int* bitstream);
typedef ogg_int64_t (*LPOVPCMTOTAL)(OggVorbis_File* vf, int i);
typedef vorbis_info* (*LPOVINFO)(OggVorbis_File* vf, int link);
typedef int (*LPOVOPENCALLBACKS)(void* datasource, OggVorbis_File* vf, char* initial, long ibytes, ov_callbacks callbacks);
typedef ogg_int64_t (*LPOVPCMTELL)(OggVorbis_File* vf);
typedef int (*LPOVPCMSEEKLAP)(OggVorbis_File*vf, ogg_int64_t pos);

LPOVCLEAR			gFnOvClear = nullptr;
LPOVREAD			gFnOvRead = nullptr;
LPOVPCMTOTAL		gFnOvPcmTotal = nullptr;
LPOVINFO			gFnOvInfo = nullptr;
LPOVOPENCALLBACKS	gFnOvOpenCallbacks = nullptr;
LPOVPCMTELL			gFnOvPcmTell = nullptr;
LPOVPCMSEEKLAP		gFnOvPcmSeekLap = nullptr;

bool gVorbisInited = false;

void initVorbis()
{
	if(gVorbisInited)
		return;

#ifdef MCD_WIN
	// Try and load Vorbis DLLs (VorbisFile.dll will load ogg.dll and vorbis.dll)
	if(HMODULE h = ::LoadLibraryA("MCDVorbis.dll"))
	{
		gFnOvClear = (LPOVCLEAR)GetProcAddress(h, "ov_clear");
		gFnOvRead = (LPOVREAD)GetProcAddress(h, "ov_read");
		gFnOvPcmTotal = (LPOVPCMTOTAL)GetProcAddress(h, "ov_pcm_total");
		gFnOvInfo = (LPOVINFO)GetProcAddress(h, "ov_info");
		gFnOvOpenCallbacks = (LPOVOPENCALLBACKS)GetProcAddress(h, "ov_open_callbacks");
		gFnOvPcmTell = (LPOVPCMTELL)GetProcAddress(h, "ov_pcm_tell");
		gFnOvPcmSeekLap = (LPOVPCMSEEKLAP)GetProcAddress(h, "ov_pcm_seek_lap");
	}
#else
	gFnOvClear = ov_clear;
	gFnOvRead = ov_read;
	gFnOvPcmTotal = ov_pcm_total;
	gFnOvInfo = ov_info;
	gFnOvOpenCallbacks = ov_open_callbacks;
	gFnOvPcmTell = ov_pcm_tell;
	gFnOvPcmSeekLap = ov_pcm_seek_lap;
#endif

	if (gFnOvClear && gFnOvRead && gFnOvPcmTotal && gFnOvInfo &&
		gFnOvOpenCallbacks && gFnOvPcmTell && gFnOvPcmSeekLap)
	{
		gVorbisInited = true;
	}
}

// Wrap around std::iostream into Ogg's callback
size_t ov_read_func(void* ptr, size_t eleSize, size_t count, void* userData)
{
	std::istream* is = reinterpret_cast<std::istream*>(userData);
	// NOTE: is->read() + is->gcount() didn't work as expected.
	// use the underlaying buffer is more reliable.
	return static_cast<size_t>(is->rdbuf()->sgetn((char*)ptr, eleSize * count));
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
	return static_cast<long>(is->tellg());
}

void swap(short& s1, short& s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

size_t gDecodeOggVorbis(OggVorbis_File* psOggVorbisFile, char* pDecodeBuffer, size_t ulBufferSize, size_t ulChannels)
{
	unsigned long ulSamples;
	unsigned long ulBytesDone = 0;

	while(true)
	{
		long decodeSize = gFnOvRead(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, nullptr);

		if(decodeSize <= 0)
			break;

		ulBytesDone += decodeSize;
		if (ulBytesDone >= ulBufferSize)
			break;
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (ulChannels == 6)
	{
		short* pSamples = (short*)pDecodeBuffer;
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
	size_t channel = info->channels * 2;

	// TODO: The operation may cause overflow
	// Frequency * 2 (16bit) * fraction of a second
	bufferInByte = info->rate * channel * bufferDurationMs / 1000;
	// Important: The buffer Size must be an exact multiple of the block alignment.
	bufferInByte -= (bufferInByte % channel);

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

size_t parseSubBufferLength(const char* args)
{
	// Returns 250ms by default
	size_t len = 250;

	if(!args)
		return len;

	NvpParser parser(args);
	const char* name, *value;
	while(parser.next(name, value))
	{
		if(strCaseCmp(name, "subBufferLength") == 0) {
			len = str2IntWithDefault(value, len);
			break;
		}
	}

	return len < 10 ? 10 : len;
}

}	// namespace

class OggLoader::Impl
{
public:
	struct Task
	{
		int bufferIdx;
		uint64_t pcmOffset;
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

			Task ret = { -1, 0, nullptr };
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
		: mHeaderLoaded(false), mIStream(nullptr)
		, mCurrentPcmOffset(0)
	{
		::memset(&mInfo, 0, sizeof(mInfo));
	}

	~Impl()
	{
		if(mHeaderLoaded)
			gFnOvClear(&mOggFile);
	}

	bool loadHeader(std::istream* is, const char* args)
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
		}

		mBufferSize = calculateBufferSize(parseSubBufferLength(args), mVorbisInfo);

		if(mBufferSize == 0)
			return false;

		format = getFormat(mVorbisInfo);
		frequency = mVorbisInfo->rate;
		mInfo.channelCount = mVorbisInfo->channels;
		mInfo.frequency = frequency;
		mInfo.totalPcm = gFnOvPcmTotal(&mOggFile, -1);

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

			MCD_ASSERT(mBufferSize > 0);
			mTmpBuf.resize(mBufferSize);

			AudioBuffer& buffer = *task.buffer;
			size_t bytesWritten;
			uint64_t pcmTell;

			{	// Protect against the seek(pcmOffset) function
				ScopeLock lock(mSeekMutex);
				bytesWritten = gDecodeOggVorbis(&mOggFile, &mTmpBuf[0], mBufferSize, mVorbisInfo->channels);
				pcmTell = gFnOvPcmTell(&mOggFile);
			}

			if(bytesWritten > 0) {
				// NOTE: Luckly that the OpenAl function alBufferData is thread safe.
				// NOTE: Make sure no other thread is using this buffer HANDLE, otherwise we
				// need to move the alBufferData() function to do inside commit().
				alBufferData(buffer.handles[task.bufferIdx], format, &mTmpBuf[0], bytesWritten, frequency);

				Task task2 = { task.bufferIdx, pcmTell, nullptr };
				mCommitQueue.push(task2);

				if(!checkAndPrintError("alBufferData failed: "))
					return -1;
			}

			// Eof
			if(bytesWritten < mBufferSize)
				return 0;
		}

		return 1;
	}

	int popLoadedBuffer()
	{
		Task task = mCommitQueue.pop();
		ScopeLock lock(mMutex);

		if(task.bufferIdx != -1)
			mCurrentPcmOffset = task.pcmOffset;

		return task.bufferIdx;
	}

	bool seek(uint64_t pcmOffset)
	{
		{	ScopeLock lock(mMutex);
			if(!mHeaderLoaded || !mOggFile.seekable)
				return false;
		}

		ScopeLock lock(mSeekMutex);
		int result = gFnOvPcmSeekLap(&mOggFile, pcmOffset);

		return result == 0;
	}

	size_t mBufferSize;	//!< Calculated suitable buffer size for each buffer in AudioBuffer.
	ALenum format;
	ALsizei frequency;
	Mutex mMutex, mSeekMutex;

	TaskQueue mTaskQueue, mCommitQueue;

	// Information about the Ogg file
	bool mHeaderLoaded;
	OggVorbis_File mOggFile;
	ov_callbacks mOggFileCallbacks;
	vorbis_info* mVorbisInfo;
	std::istream* mIStream;
	IAudioStreamLoader::Info mInfo;
	uint64_t mCurrentPcmOffset;	//!< The PCM

	std::vector<char> mTmpBuf;
};	// Impl

OggLoader::OggLoader()
	: mImpl(*new Impl)
{
	initVorbis();
}

OggLoader::~OggLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState OggLoader::load(std::istream* is, const Path*, const char* args)
{
	ScopeLock lock(mImpl.mMutex);

	if(!is || !mImpl.loadHeader(is, args))
		return Aborted;

	// NOTE: The mutex will unlock for a while in mImpl.loadData().
	int result = mImpl.loadData();

	// NOTE: Differ from other loaders, this streamming loader will never return
	// IResourceLoader::Loaded, in favour of audio seeking/looping.
	return result == -1 ? Aborted : PartialLoaded;
}

void OggLoader::commit(Resource& resource)
{
	// Currently nothing to do inside commit
}

// Invoked by user or AudioSource
void OggLoader::requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex)
{
	Impl::Task taks = { int(bufferIndex), 0, buffer };
	mImpl.mTaskQueue.push(taks);
	continueLoad();
}

int OggLoader::popLoadedBuffer()
{
	return mImpl.popLoadedBuffer();
}

IAudioStreamLoader::Info OggLoader::info() const
{
	ScopeLock lock(mImpl.mMutex);
	return mImpl.mInfo;
}

uint64_t OggLoader::pcmOffset() const
{
	ScopeLock lock(mImpl.mMutex);
	return mImpl.mCurrentPcmOffset;
}

bool OggLoader::seek(uint64_t pcmOffset)
{
	{	ScopeLock lock(mImpl.mMutex);
		if(loadingState() & IResourceLoader::Stopped)
			return false;
	}

	return mImpl.seek(pcmOffset);
}

ResourcePtr OggLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "ogg") != 0)
		return nullptr;

	int bufferCount = AudioBuffer::cMaxBuffers;

	if(args) {
		NvpParser parser(args);
		const char* name, *value;
		while(parser.next(name, value))
		{
			if(strCaseCmp(name, "bufferCount") == 0) {
				bufferCount = str2IntWithDefault(value, AudioBuffer::cMaxBuffers);
				break;
			}
		}

		if(bufferCount < 1)
			bufferCount = 1;
		if(bufferCount > AudioBuffer::cMaxBuffers)
			bufferCount = AudioBuffer::cMaxBuffers;
	}

	return new AudioBuffer(fileId, bufferCount);
}

IResourceLoaderPtr OggLoaderFactory::createLoader()
{
	return new OggLoader;
}

}	// namespace MCD
