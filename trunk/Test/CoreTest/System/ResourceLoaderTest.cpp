#include "Pch.h"
#include "../../../SGE/Core/System/Resource.h"
#include "../../../SGE/Core/System/ResourceLoader.h"
#include <vector>

using namespace SGE;

namespace {

class FakeResource : public Resource
{
public:
	explicit FakeResource(const Path& fileId) : Resource(fileId) {}
	std::vector<int> data;
};	// FakeResource

class FakeResourceLoader : public IResourceLoader
{
public:
	FakeResourceLoader() : mLoadingState(NotLoaded), mCommited(false) {}

	sal_override LoadingState load(sal_maybenull std::istream*)
	{
		mBuffer.push_back(1);
		return (mLoadingState = Loaded);
	}

	sal_override void commit(Resource& resource)
	{
		if(mCommited)
			return;
		FakeResource& res = static_cast<FakeResource&>(resource);
		res.data.swap(mBuffer);
		mCommited = true;
	}

	sal_override LoadingState getLoadingState() const { return mLoadingState; }

protected:
	LoadingState mLoadingState;
	std::vector<int> mBuffer;
	bool mCommited;
};	// FakeResourceLoader

}	// namespace

TEST(Basic_ResourceLoaderTest)
{
	FakeResource resource(L"./myResource.bin");
	FakeResourceLoader loader;

	CHECK_EQUAL(IResourceLoader::NotLoaded, loader.getLoadingState());

	CHECK_EQUAL(IResourceLoader::Loaded, loader.load(nullptr));
	loader.commit(resource);

	// The second commit is ignonred for this loader
	loader.commit(resource);

	CHECK_EQUAL(1u, resource.data.size());
}

#include "../../../SGE/Core/System/Thread.h"
#include "../../../SGE/Core/System/CondVar.h"

namespace {

class FakeAsyncResourceLoader : public IResourceLoader
{
public:
	FakeAsyncResourceLoader()
		:
		mLoadingState(NotLoaded),
		mIteration(0)
	{}

	sal_override LoadingState load(sal_maybenull std::istream*)
	{
		ScopeLock lock(mMutex);
		if(++mIteration > cIterationCount)
			return (mLoadingState = Loaded);

		mBuffer.push_back(mIteration);
		return (mLoadingState = PartialLoaded);
	}

	sal_override void commit(Resource& resource)
	{
		ScopeLock lock(mMutex);
		FakeResource& res = static_cast<FakeResource&>(resource);
		std::copy(mBuffer.begin(), mBuffer.end(), std::back_inserter(res.data));
		mBuffer.clear();
	}

	sal_override LoadingState getLoadingState() const
	{
		ScopeLock lock(mMutex);
		return mLoadingState;
	}

	static const size_t cIterationCount = 100;

protected:
	IResourceLoader::LoadingState mLoadingState;
	std::vector<int> mBuffer;
	size_t mIteration;
	mutable Mutex mMutex;
};	// FakeAsyncResourceLoader

}	// namespace

TEST(Async_ResourceLoaderTest)
{
	class Runnable : public Thread::IRunnable
	{
	public:
		Runnable(FakeAsyncResourceLoader& loader) : mLoader(loader) {}

		void wait() {
			mCondVar.wait();
		}

	protected:
		sal_override void run(Thread& thread) throw()
		{
			while(thread.keepRun()) {
				if(mLoader.load(nullptr) & IResourceLoader::Stopped) {
					mCondVar.signal();
					return;
				}
			}
		}

		FakeAsyncResourceLoader& mLoader;
		CondVar mCondVar;
	};	// Runnable

	FakeResource resource(L"./myResource.bin");
	FakeAsyncResourceLoader loader;

	CHECK_EQUAL(IResourceLoader::NotLoaded, loader.getLoadingState());

	Runnable runnable(loader);
	Thread thread(runnable, false);

	while(!(loader.getLoadingState() & IResourceLoader::Stopped))
		runnable.wait();

	CHECK_EQUAL(IResourceLoader::Loaded, loader.getLoadingState());
}
