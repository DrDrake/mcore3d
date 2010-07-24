#include "Pch.h"
#include "SkeletonLoader.h"
#include "Skeleton.h"
#include "../System/MemoryProfiler.h"
#include "../System/Stream.h"
#include <iostream>

namespace MCD {

class SkeletonLoader::Impl
{
public:
	Impl() : skeleton(new Skeleton("tmp")) {}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	SkeletonPtr skeleton;
	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

IResourceLoader::LoadingState SkeletonLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	// Simplying the error check
	#define ABORT_IF(expression) if(expression) { MCD_ASSERT(false); return mLoadingState = Aborted; }

	ABORT_IF(!is || !skeleton);

	if(mLoadingState != Loading)
		mLoadingState = NotLoaded;

	if(mLoadingState & Stopped)
		return mLoadingState;

	uint16_t jointCount;
	ABORT_IF(!MCD::read(*is, jointCount));
	skeleton->init(jointCount);

	for(size_t i=0; i<jointCount; ++i) {
		uint16_t parent;
		ABORT_IF(!MCD::read(*is, parent));
		skeleton->parents[i] = parent;
		ABORT_IF(!MCD::readString(*is, skeleton->names[i]));
	}

	return mLoadingState = Loaded;

	#undef ABORT_IF
}

void SkeletonLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because SkeletonLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.
	Skeleton& s = dynamic_cast<Skeleton&>(resource);
	skeleton->swap(s);

	// Our temporary skeleton object is no longer needed.
	skeleton = nullptr;
}

SkeletonLoader::SkeletonLoader()
	: mImpl(*new Impl)
{
}

SkeletonLoader::~SkeletonLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState SkeletonLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("SkeletonLoader::load");
	return mImpl.load(is, fileId, args);
}

void SkeletonLoader::commit(Resource& resource)
{
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState SkeletonLoader::getLoadingState() const
{
	return mImpl.mLoadingState;
}

}	// namespace MCD
