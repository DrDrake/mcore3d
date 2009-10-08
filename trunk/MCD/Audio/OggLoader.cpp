#include "Pch.h"
#include "OggLoader.h"
#include "AudioBuffer.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PlatformInclude.h"
#include "../Core/System/ResourceManager.h"
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/VorbisOgg/vorbisfile.h"

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
	is->read((char*)ptr, eleSize * count);
	return is->gcount();
}

int ov_seek_func(void* userData, ogg_int64_t offset, int whence)
{
		return -1;
/*	std::istream* is = reinterpret_cast<std::istream*>(userData);
	if(whence == SEEK_SET)
		is->seekg(long(offset), std::ios_base::beg);
	else if(whence == SEEK_CUR)
		is->seekg(long(offset), std::ios_base::cur);
	else if(whence == SEEK_END)
		is->seekg(long(offset), std::ios_base::end);
	else
		return -1;

	return (is->eof() || is->bad()) ? -1 : 0;*/
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

}	// namespace

class OggLoader::Impl
{
public:
	ALenum format;
	ALsizei dataSize;
	ALsizei frequency;
	char* data;
	Mutex mMutex;
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
		loadingState = Aborted;
	else if(loadingState == Aborted)
		loadingState = NotLoaded;

//	if(loadingState & Stopped)
//		return loadingState;

	ov_callbacks callbacks;
	callbacks.read_func = ov_read_func;
	callbacks.seek_func = ov_seek_func;
	callbacks.close_func = ov_close_func;
	callbacks.tell_func = ov_tell_func;

	OggVorbis_File oggVorbisFile;

	if(!gFnOvOpenCallbacks(is, &oggVorbisFile, nullptr, 0, callbacks))
	{
		loadingState = Aborted;
	}

	// Get some information about the file (Channels, Format, and Frequency)
	if(vorbis_info* psVorbisInfo = gFnOvInfo(&oggVorbisFile, -1))
	{
		size_t bufferSize = psVorbisInfo->rate;
		bufferSize -= bufferSize % 2;
		(void)bufferSize;

		// TODO: Check success
		void* buf = malloc(bufferSize);

		size_t bytesWritten = gDecodeOggVorbis(&oggVorbisFile, (char*)buf, bufferSize, psVorbisInfo->channels);
		(void)bytesWritten;
	}

	int result;
	{	// There is no need to do a mutex lock during loading, since
		// no body can access the mImageData if the loading isn't finished.

		ScopeUnlock unlock(mImpl->mMutex);

		// TODO: Actual load from wave file
//		char* bufferData = nullptr;
		result = 0;
	}

	return (loadingState = (result == 0) ? Loaded : Aborted);
}

void OggLoader::commit(Resource& resource)
{
	// Will throw exception if the resource is not of the type AudioBuffer
	AudioBuffer& audioBuffer = dynamic_cast<AudioBuffer&>(resource);

	for(size_t i=0; i<audioBuffer.bufferCount(); ++i)
		alBufferData(audioBuffer.handles[i], mImpl->format, mImpl->data, mImpl->dataSize, mImpl->frequency);

//	if(alGetError() == AL_NO_ERROR)
//		bReturn = AL_TRUE;
}

IResourceLoader::LoadingState OggLoader::getLoadingState() const
{
	// We don't have arithmetics on loadingState, so we don't need to lock on it
	return loadingState;
}

void OggLoader::onPartialLoaded(IResourceManager& manager, void* context, uint priority, const wchar_t* args)
{
	manager.reSchedule(context, priority, args);
}

void OggLoader::requestLoad(const AudioBufferPtr& buffer, size_t bufferIndex)
{
}

}	// namespace MCD
