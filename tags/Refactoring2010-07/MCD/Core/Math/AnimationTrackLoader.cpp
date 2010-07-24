#include "Pch.h"
#include "AnimationTrackLoader.h"
#include "AnimationTrack.h"
#include "../System/MemoryProfiler.h"
#include "../System/Stream.h"
#include <iostream>

namespace MCD {

class AnimationTrackLoader::Impl
{
public:
	Impl() : track(new AnimationTrack("tmp")) {}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	AnimationTrackPtr track;
	std::vector<size_t> keyframeCount;
	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

IResourceLoader::LoadingState AnimationTrackLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	// Simplying the error check
	#define ABORT_IF(expression) if(expression) { MCD_ASSERT(false); return mLoadingState = Aborted; }

	ABORT_IF(!is || !track);

	if(mLoadingState != Loading)
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

	AnimationTrack::ScopedWriteLock lock(*track);

	ABORT_IF(!track->init(StrideArray<const size_t>(&keyframeCount[0], subtrackCount)));
	ABORT_IF(!MCD::read(*is, track->loop));
	ABORT_IF(!MCD::read(*is, track->naturalFramerate));

	AnimationTrack::Subtracks& st = track->subtracks;
	AnimationTrack::KeyFrames& kf = track->keyframes;
	ABORT_IF(MCD::read(*is, st.getPtr(), st.sizeInByte()) != std::streamsize(st.sizeInByte()));
	ABORT_IF(MCD::read(*is, kf.getPtr(), kf.sizeInByte()) != std::streamsize(kf.sizeInByte()));
	ABORT_IF(!track->checkValid());

	return mLoadingState = Loaded;

	#undef ABORT_IF
}

void AnimationTrackLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because AnimationTrackLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.
	AnimationTrack& t = dynamic_cast<AnimationTrack&>(resource);
	track->swap(t);

	// Our temporary track object is no longer needed.
	track = nullptr;
}

AnimationTrackLoader::AnimationTrackLoader()
	: mImpl(*new Impl)
{
}

AnimationTrackLoader::~AnimationTrackLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState AnimationTrackLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("AnimationTrackLoader::load");
	return mImpl.load(is, fileId, args);
}

void AnimationTrackLoader::commit(Resource& resource)
{
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState AnimationTrackLoader::getLoadingState() const
{
	return mImpl.mLoadingState;
}

}	// namespace MCD
