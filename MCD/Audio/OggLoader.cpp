#include "Pch.h"
#include "OggLoader.h"
#include "AudioBuffer.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PlatformInclude.h"
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

int ov_seek_func(void* datasource, ogg_int64_t offset, int whence)
{
	return 0;
}

int ov_close_func(void* datasource)
{
	return 0;
}

long ov_tell_func(void* datasource)
{
	return 0;
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

	if(loadingState & Stopped)
		return loadingState;

	ov_callbacks callbacks;
	callbacks.read_func = ov_read_func;
	callbacks.seek_func = ov_seek_func;
	callbacks.close_func = ov_close_func;
	callbacks.tell_func = ov_tell_func;

//	OggVorbis_File oggVorbisFile;

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
	// Will throw exception if the resource is not of the type Texture
	AudioBuffer& audioBuffer = dynamic_cast<AudioBuffer&>(resource);

	alBufferData(audioBuffer.handle, mImpl->format, mImpl->data, mImpl->dataSize, mImpl->frequency);

//	if(alGetError() == AL_NO_ERROR)
//		bReturn = AL_TRUE;
}

IResourceLoader::LoadingState OggLoader::getLoadingState() const
{
	// We don't have arithmetics on loadingState, so we don't need to lock on it
	return loadingState;
}

}	// namespace MCD
