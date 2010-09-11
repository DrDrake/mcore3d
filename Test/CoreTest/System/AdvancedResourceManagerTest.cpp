#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/Resource.h"
#include "../../../MCD/Core/System/ResourceLoader.h"
#include "../../../MCD/Core/System/ResourceManager.h"

using namespace MCD;

// Test the loader controled delaying of progressive load. The most noticable 
// application is the use in Texture and Audio streamming.

namespace {

class DelayLoader : public IResourceLoader
{
public:
	DelayLoader() : mLoadCount2(0) {}

	size_t getLoadCount() const
	{
		ScopeLock lock(mMutex);
		return mLoadCount2;
	}

protected:
	sal_override sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_maybenull const char* args=nullptr)
	{
		// For the testing purpose, we ignore the "is" pointer.
		if(!is && false)
			return Aborted;

		// Do actual loading...

		{	// We pretenting all load will finish after 10 iterations
			ScopeLock lock(mMutex);
			if(++mLoadCount2 == 10)
				return Loaded;

			MCD_ASSERT(mLoadCount2 <= 10);
		}

		continueLoad();

		return PartialLoaded;
	}

	sal_override void commit(Resource&) {}

	size_t mLoadCount2;		//!< For test purpose
};	// DelayLoader

class DelayLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const char* args) {
		return new Resource(fileId);
	}

	sal_override IResourceLoaderPtr createLoader() {
		return new DelayLoader;
	}
};	// DelayLoaderFactory

class DelayLoaderTestFixture
{
protected:
	DelayLoaderTestFixture(int blockIteration)
		: fs(new RawFileSystem("./")), manager(*fs)
	{
		manager.addFactory(new DelayLoaderFactory);
		(void)manager.load("test", blockIteration, 0, nullptr);
		loader = manager.getLoader("test");
	}

	RawFileSystem* fs;
	ResourceManager manager;
	IResourceLoaderPtr loader;
};	// DelayLoaderTestFixture

}	// namespace

TEST_FIXTURE_CTOR(DelayLoaderTestFixture, (0), InitialNoBlock)
{
	while(loader->loadingState() != IResourceLoader::Loaded) {
		IResourceLoaderPtr l = manager.popEvent();
		if(!loader) continue;
	}

	CHECK_EQUAL(10u, dynamic_cast<DelayLoader&>(*loader).getLoadCount());
}

TEST_FIXTURE_CTOR(DelayLoaderTestFixture, (2), InitialBlockSome)
{
	while(loader->loadingState() != IResourceLoader::Loaded) {
		IResourceLoaderPtr l = manager.popEvent();
		if(!loader) continue;
	}

	CHECK_EQUAL(10u, dynamic_cast<DelayLoader&>(*loader).getLoadCount());
}
