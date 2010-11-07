#include "Pch.h"
#include "AnimationTrackLoader.h"
#include "AnimationTrack.h"
#include "../System/MemoryProfiler.h"
#include "../System/Stream.h"
#include <iostream>
#include <vector>

namespace MCD {

class AnimationClipLoader::Impl
{
public:
	Impl() : track(new AnimationClip("tmp")) {}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	AnimationClipPtr track;
	std::vector<size_t> keyframeCount;
	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

IResourceLoader::LoadingState AnimationClipLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	// Simplying the error check
	#define ABORT_IF(expression) if(expression) { MCD_ASSERT(false); return mLoadingState = Aborted; }

	ABORT_IF(!is || !track);

/*	if(mLoadingState != Loading)
		mLoadingState = NotLoaded;

	if(mLoadingState & Stopped)
		return mLoadingState;

	uint16_t subtrackCount;
	ABORT_IF(!MCD::read(*is, subtrackCount));
	keyframeCount.resize(subtrackCount);

	for(size_t i=0; i<subtrackCount; ++i) {
		uint32_t count;
		ABORT_IF(!MCD::read(*is, count));
		keyframeCount[i] = count;
	}

	AnimationClip::ScopedWriteLock lock(*track);

	ABORT_IF(!track->init(StrideArray<const size_t>(&keyframeCount[0], subtrackCount)));
	ABORT_IF(!MCD::read(*is, track->loop));
	ABORT_IF(!MCD::read(*is, track->naturalFramerate));

	AnimationClip::Subtracks& st = track->subtracks;
	AnimationClip::KeyFrames& kf = track->keyframes;
	ABORT_IF(MCD::read(*is, st.getPtr(), st.sizeInByte()) != std::streamsize(st.sizeInByte()));
	ABORT_IF(MCD::read(*is, kf.getPtr(), kf.sizeInByte()) != std::streamsize(kf.sizeInByte()));
	ABORT_IF(!track->checkValid());
*/
	return mLoadingState = Loaded;

	#undef ABORT_IF
}

void AnimationClipLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because AnimationClipLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.
	AnimationClip& t = dynamic_cast<AnimationClip&>(resource);
	track->swap(t);

	// Our temporary track object is no longer needed.
	track = nullptr;
}

AnimationClipLoader::AnimationClipLoader()
	: mImpl(*new Impl)
{
}

AnimationClipLoader::~AnimationClipLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState AnimationClipLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("AnimationClipLoader::load");
	return mImpl.load(is, fileId, args);
}

void AnimationClipLoader::commit(Resource& resource)
{
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState AnimationClipLoader::getLoadingState() const
{
	return mImpl.mLoadingState;
}

}	// namespace MCD
