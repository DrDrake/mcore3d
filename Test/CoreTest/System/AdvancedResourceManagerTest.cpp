#include "Pch.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include "../../../MCD/Core/System/Resource.h"
#include "../../../MCD/Core/System/ResourceLoader.h"
#include "../../../MCD/Core/System/ResourceManager.h"

using namespace MCD;

// Test the loader controled delaying of progressive load. The best application is
// the use in Audio streamming.

namespace {

class DelayLoader : public IResourceLoader
{
public:
	DelayLoader()
		: mState(NotLoaded), mLoadCount(0), mRequestPending(false)
		, mPartialLoadContext(nullptr), mResourceManager(nullptr)
	{}

	void requestLoad()
	{
		ScopeLock lock(mMutex);

		// If mPartialLoadContext is null, a loading is already in progress,
		// onPartialLoaded() will be invoked soon, therefore we just pend the request.
		if(!mResourceManager || !mPartialLoadContext) {
			mRequestPending = true;
			return;
		}

		// Otherwise we can tell resource manager to re-schedule the load immediatly.
		mResourceManager->reSchedule(mPartialLoadContext, 0, nullptr);
		mResourceManager = nullptr;
		mPartialLoadContext = nullptr;
	}

	size_t getLoadCount() const
	{
		ScopeLock lock(mMutex);
		return mLoadCount;
	}

	void abortLoad()
	{
		{	ScopeLock lock(mMutex);
			mState = Aborted;
		}

		// Notify the resource manager to trigger IResourceLoader::load(),
		// so that all stuffs will be clear because of the Abort state.
		requestLoad();
	}

protected:
	sal_override sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_maybenull const wchar_t* args=nullptr)
	{
		ScopeLock lock(mMutex);

		// For the testing purpose, we ignore the "is" pointer.
		if(!is && false)
			return mState = Aborted;

		{	ScopeUnlock unlock(mMutex);
			// Do actual loading...
		}

		mState = PartialLoaded;

		// We pretenting all load will finish after 10 iterations
		if(++mLoadCount == 10)
			mState = Loaded;

		return mState;
	}

	sal_override void commit(Resource&) {}

	sal_override LoadingState getLoadingState() const
	{
		ScopeLock lock(mMutex);
		return mState;
	}

	sal_override void onPartialLoaded(IResourceManager& manager, void* context, uint priority, const wchar_t* args)
	{
		ScopeLock lock(mMutex);

		if(mRequestPending) {
			manager.reSchedule(context, priority, args);
			mRequestPending = false;
		} else {
			mPartialLoadContext = context;
			mResourceManager = &manager;
		}
	}

	volatile LoadingState mState;
	size_t mLoadCount;		//!< For test purpose
	bool mRequestPending;	//!< Indicate a continue load request is pending or not
	void* mPartialLoadContext;
	IResourceManager* mResourceManager;
	mutable Mutex mMutex;
};	// FakeLoader

class DelayLoaderFactory : public ResourceManager::IFactory
{
public:
	sal_override ResourcePtr createResource(const Path& fileId, const wchar_t* args) {
		return new Resource(L"");
	}

	sal_override IResourceLoader* createLoader() {
		return new DelayLoader;
	}
};	// DelayLoaderFactory

class DelayLoaderTestFixture
{
protected:
	DelayLoaderTestFixture(IResourceManager::BlockingMode blockingMode)
		: fs(new RawFileSystem(L"./")), manager(*fs)
	{
		manager.addFactory(new DelayLoaderFactory);
		res = manager.load(L"", blockingMode,	0, nullptr, &_loader);
		loader = dynamic_cast<DelayLoader*>(_loader.get());
	}

	~DelayLoaderTestFixture()
	{
		// NOTE: Be sure to abort the load before everythings shutdown, otherwise memory leak,
		// because we use void* as the partial load context.
		loader->abortLoad();
	}

	RawFileSystem* fs;
	ResourceManager manager;
	ResourcePtr res;
	IResourceLoaderPtr _loader;
	DelayLoader* loader;
};	// OggTestFixture

}	// namespace

// If we didn't actively request the loader to continue to load, the
// load will never finish.
TEST_FIXTURE_CTOR(DelayLoaderTestFixture, (IResourceManager::FirstPartialBlock), NoRequest)
{
	CHECK(loader);

	size_t testCount = 100;
	while(testCount--) {
		ResourceManager::Event event = manager.popEvent();
		CHECK_EQUAL(1u, loader->getLoadCount());
	}

	CHECK(loader);
}
