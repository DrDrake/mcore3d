#include "Pch.h"
#include "ResourceManager.h"
#include "CondVar.h"
#include "Deque.h"
#include "FileSystem.h"
#include "Log.h"
#include "Macros.h"
#include "MemoryProfiler.h"
#include "PtrVector.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "TaskPool.h"
#include "Timer.h"
#include "Utility.h"
#include <algorithm>	// for std::find

namespace MCD {

namespace {

class DummyLoader : public IResourceLoader
{
public:
	LoadingState load(std::istream* is, const Path* fileId, const char* args) { return Loaded; }
	sal_override void commit(Resource&) {}
};	// DummyLoader

}	// namespace

class ResourceManager::Impl
{
	//! A thread safe event queue to make communication between the main thread and loader thread
	class EventQueue
	{
	public:
		// This function is called inside the worker thread, and the mutex should already be locked
		void pushBackNoLock(const IResourceLoaderPtr& event)
		{
			MCD_ASSUME(mMutex && mMutex->isLocked());
			// Each instance of resource should appear in the event queue once.
			// Note: Linear search is enough since the event queue should keep consumed every frame
			MCD_FOREACH(const IResourceLoaderPtr e, mQueue) {
				if(e == event)
					return;
			}

			mQueue.push_back(event);

			// NOTE: Keep the life of the Resource till at least popEvent()
			if(Resource* r = event->resource().get())
				intrusivePtrAddRef(r);

			mMutex->broadcastNoLock();
		}

		IResourceLoaderPtr popFrontNoLock()
		{
			MCD_ASSUME(mMutex && mMutex->isLocked());
			IResourceLoaderPtr ret = nullptr;
			if(!mQueue.empty()) {
				ret = mQueue.front();

				if(Resource* r = ret->resource().get())
					intrusivePtrRelease(r);

				mQueue.pop_front();
				mMutex->broadcastNoLock();
			}
			return ret;
		}

		void pushBack(const IResourceLoaderPtr& event)
		{
			MCD_ASSUME(mMutex);
			ScopeLock lock(*mMutex);
			pushBackNoLock(event);
		}

		IResourceLoaderPtr popFront()
		{
			MCD_ASSUME(mMutex);
			ScopeLock lock(*mMutex);
			return popFrontNoLock();
		}

		sal_notnull CondVar* mMutex;
		std::deque<IResourceLoaderPtr> mQueue;
	};	// EventQueue

public:
	Impl(TaskPool* externalTaskPool, ResourceManager& manager, IFileSystem& fileSystem, bool takeFileSystemOwnership)
		: mTaskPool(externalTaskPool)
		, mBackRef(manager)
		, mFileSystem(fileSystem)
		, mTakeFileSystemOwnership(takeFileSystemOwnership)
		, mCreatorThreadId(getCurrentThreadId())
	{
		mIsExternalTaskPool = externalTaskPool != nullptr;
		if(!externalTaskPool)
			mTaskPool.reset(new TaskPool);
		mEventQueue.mMutex = &mMutex;
	}

	~Impl()
	{
		// Stop the task pool first
		// Tasks may be invoked in this thread context and so they
		// may try to acquire mMutex. Acquiring the mutex
		// before calling task pool stop will result a dead lock.
		mTaskPool->stop();

		{	ScopeLock lock(mMutex);
			if(mTakeFileSystemOwnership)
				delete &mFileSystem;
			removeAllFactory();
		}

		if(mIsExternalTaskPool)
			mTaskPool.release();
	}

	IResourceLoaderPtr findCache(const Path& fileId)
	{
		return mResourceMap.find(fileId)->getOuterSafe();
	}

	void addCache(const IResourceLoaderPtr& cache)
	{
		MCD_VERIFY(mResourceMap.insertUnique(cache->mPathKey));
		intrusivePtrAddRef(cache.get());
	}

	/// Block until the required load count is reached and committed.
	void block(const Path& fileId, IResourceLoader& loader, Resource& resource, int blockIteration, int priority, sal_in_z const char* args)
	{
		MCD_ASSERT(blockIteration >= 0);
		if(blockIteration == 0)
			return;

		bool hasWorkDone = false;
		{	ScopeLock lock(loader.mMutex);
			while(int(loader.mLoadCount) < blockIteration && !(loader.mState & IResourceLoader::Stopped))
			{
				loader.mNeedEnqueu = (int(loader.mLoadCount) == blockIteration - 1);	// Prevent continueLoad() enqueue task
				loader._load(&fileId, args);
				hasWorkDone = true;
			}

			if(!hasWorkDone)
				return;

			loader.mPendForCommit = true;
		}

		mEventQueue.pushBack(&loader);

		// Only call commit() if we where in main thread (the ResourceManager's creator thread).
		if(mCreatorThreadId == getCurrentThreadId()) {
			ScopeLock lock(loader.mMutex);
			loader.mPendForCommit = false;	// Just put in event queue but no call to commit() as we will do it right here.
			commitResource(loader);
		}
		// Poll to see if the target load count reached and committed
		else {
			ScopeLock lock(loader.mMutex);
			while(int(loader.mLoadCount) < blockIteration || resource.commitCount() == 0 && loader.mState != IResourceLoader::Aborted) {
				ScopeUnlock unlock(loader.mMutex);
				mMutex.wait();
			}
		}
	}

	ResourcePtr actualLoad(const Path& fileId, IResourceLoader& loader, int blockIteration, int priority, sal_in_z const char* args)
	{
		const ResourcePtr ret = loader.resource();
		MCD_ASSERT(ret);

		// NOTE: We should set it's priority BEFORE pushing into the task pool (continueLoad()).
		loader.setPriority(priority);

		// Get the default blockIteration from the loader
		const int forceBlockingIteration = loader.forceBlockingIteration();
		if(forceBlockingIteration != -1)
			blockIteration = forceBlockingIteration;
		if(blockIteration < 0)
			blockIteration = loader.defaultBlockingIteration();

		// Simple let it load in taskPool
		MCD_ASSERT(blockIteration >= 0);
		if(blockIteration == 0)
			loader.continueLoad();
		// Block until a user specified count of iteration had processed
		else
			block(fileId, loader, *ret, blockIteration, priority, args);

		return ret;
	}

	void addFactory(IFactory* factory)
	{
		MCD_ASSERT(mMutex.isLocked());
		mFactories.push_back(factory);
	}

	void removeAllFactory()
	{
		MCD_ASSERT(mMutex.isLocked());
		mFactories.clear();
	}

	IResourceLoaderPtr createLoader(const Path& fileId, sal_in_z const char* args, ResourcePtr& resource)
	{
		MCD_ASSERT(mMutex.isLocked());
		IResourceLoaderPtr loader;

		// Loop for all factories to see which one will response to the fileId
		// NOTE: We loop the factories in reverse order, so that user can override a new type of
		// loader factory by inserting a new one.
		for(Factories::reverse_iterator i=mFactories.rbegin(); i!=mFactories.rend(); ++i) {
			if((resource = i->createResource(fileId, args))) {
				loader = i->createLoader();
				loader->mPathKey.setKey(fileId);
				loader->mArgs = args ? args : "";
				loader->mResource = resource.get();
				loader->mResourceManager = &mBackRef;
				mResourceHolder.push_back(resource);
				break;
			}
		}

		if(!loader)
			Log::format(Log::Warn, "No loader for \"%s\" can be found", fileId.c_str());

		return loader;
	}

	void commitResource(IResourceLoader& loader)
	{
		MCD_ASSERT(loader.mMutex.isLocked());

		if(loader.mState == IResourceLoader::Aborted)
			Log::format(Log::Warn, "Resource: %s %s", loader.mPathKey.getKey().c_str(), "failed to load");
		else if(ResourcePtr r = loader.mResource.lock()) {
			{	ScopeUnlock unlock(loader.mMutex);
				loader.commit(*r);
			}
			r->mCommitCount++;
			mMutex.signal();
		}
	}

public:
	std::auto_ptr<TaskPool> mTaskPool;
	bool mIsExternalTaskPool;

	Map<IResourceLoader::PathKey> mResourceMap;

	typedef ptr_vector<IFactory> Factories;
	Factories mFactories;

	EventQueue mEventQueue;

	ResourceManager& mBackRef;
	IFileSystem& mFileSystem;
	bool mTakeFileSystemOwnership;

	const int mCreatorThreadId;	/// Store which thread create this ResourceManager

	std::vector<ResourcePtr> mResourceHolder;	/// To prolong the life of a Resource till at least popEvent()

	CondVar mMutex;
};	// Impl

ResourceManager::ResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership)
{
	mImpl = new Impl(nullptr, *this, fileSystem, takeFileSystemOwnership);
}

ResourceManager::ResourceManager(IFileSystem& fileSystem, TaskPool& taskPool, bool takeFileSystemOwnership)
{
	mImpl = new Impl(&taskPool, *this, fileSystem, takeFileSystemOwnership);
}

ResourceManager::~ResourceManager()
{
	delete mImpl;
}

ResourcePtr ResourceManager::load(const Path& fileId, int blockIteration, int priority, const char* args)
{
	args = args ? args : "";
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	// Find for existing resource (Cache hit!)
	IResourceLoaderPtr cache = mImpl->findCache(fileId);
	if(cache)
	{
		// Do clean up for dead resource cache
		// It is performed right here because a resource is shared,
		// and we have no idea when the resource will be destroyed other
		// than poll for it's weak pointer periodically
		// To some extend, this is a garbage collector!
		IResourceLoaderPtr nextCache = cache->mPathKey.next()->getOuterSafe();
		if(nextCache && !nextCache->resource())
			nextCache->releaseThis();

		// Weak pointer not null, the resource is still alive
		if(const ResourcePtr p = cache->resource()) {
			const int forceBlockingIteration = cache->forceBlockingIteration();
			if(forceBlockingIteration != -1)
				blockIteration = forceBlockingIteration;
			if(blockIteration < 0)
				blockIteration = cache->defaultBlockingIteration();
			if(blockIteration == 0)
				return p;

			// Block load as requested
			ScopeUnlock unlock(mImpl->mMutex);
			mImpl->block(fileId, *cache, *p, blockIteration, priority, args);

			return p;
		}
		else {	// Unfortunately, the resource is already deleted
			cache->releaseThis();
			cache = nullptr;
		}
	}

	ResourcePtr ret;
	IResourceLoaderPtr loader = mImpl->createLoader(fileId, args, ret);

	if(!loader)
		return nullptr;

	// Cache it to the map
	if(!cache)
		mImpl->addCache(loader);

	// Now we can begin the load operation
	lock.unlockAndCancel();
	return mImpl->actualLoad(fileId, *loader, blockIteration, priority, args);
}

ResourcePtr ResourceManager::reload(const Path& fileId, int blockIteration, int priority, const char* args)
{
	args = args ? args : "";
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	// Find for existing resource
	IResourceLoaderPtr loader = mImpl->findCache(fileId);
	ResourcePtr r;

	// The resource is not found
	if(!loader || !(r = loader->resource())) {
		lock.unlockAndCancel();
		return load(fileId, blockIteration, priority, args);
	}

	// We are just interested in the loader, not the new resource
	ResourcePtr dummy;
	loader->releaseThis();
	loader = mImpl->createLoader(fileId, args, dummy);
	if(!loader)
		return nullptr;

	mImpl->addCache(loader);

	lock.unlockAndCancel();

	{	// Use the old resource
		ScopeLock lock2(loader->mMutex);
		loader->mResource = r.get();
	}

	// Now we can begin the load operation
	return mImpl->actualLoad(fileId, *loader, blockIteration, priority, args);
}

IResourceLoaderPtr ResourceManager::getLoader(const Path& fileId)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);
	return mImpl->findCache(fileId);
}

ResourcePtr ResourceManager::cache(const ResourcePtr& resource)
{
	if(!resource)
		return resource;

	if(resource->fileId().getString().empty()) {
		MCD_ASSERT(false && "It's meaningless to cache a resource without a name");
		return nullptr;
	}

	ResourcePtr ret;

	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	IResourceLoaderPtr cache = mImpl->findCache(resource->fileId());
	// Find for existing resource
	if(cache) {
		ret = cache->resource();
		cache->releaseThis();
		cache = nullptr;
	}

	cache = new DummyLoader;
	cache->mPathKey.setKey(resource->fileId());
	cache->mResource = resource.get();
	mImpl->addCache(cache);

	// Generate a finished loading event
	mImpl->mEventQueue.pushBackNoLock(cache);

	return ret;
}

ResourcePtr ResourceManager::uncache(const Path& fileId)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);

	// Find and remove the existing resource linkage from the manager
	IResourceLoaderPtr cache = mImpl->findCache(fileId);
	if(!cache)
		return nullptr;

	ResourcePtr ret = cache->resource();
	cache->releaseThis();
	return ret;
}

void ResourceManager::customLoad(const ResourcePtr& resource, const IResourceLoaderPtr& loader, int blockIteration, int priority, const char* args)
{
	{	ScopeLock lock2(loader->mMutex);
		loader->mPathKey.setKey(resource->fileId());
		loader->mResource = resource.get();
		loader->mResourceManager = this;
	}

	MCD_ASSUME(mImpl != nullptr);
	args = args ? args : "";
	mImpl->actualLoad(resource->fileId(), *loader, blockIteration, priority, args);
}

IResourceLoaderPtr ResourceManager::popEvent(Timer* timer, float timeOut, bool performLoad)
{
	if(timer && float(timer->get().asSecond()) >= timeOut)
		return nullptr;

	MCD_ASSUME(mImpl != nullptr);

	IResourceLoaderPtr loader = mImpl->mEventQueue.popFront();

	// No event generate if the resource no longer exist
	if(loader && loader->resource()) {
		ScopeLock lock(loader->mMutex);
		if(loader->mPendForCommit) {
			loader->mPendForCommit = false;
			mImpl->commitResource(*loader);
		}
	}
	else
		loader = nullptr;

	if(performLoad || mImpl->mTaskPool->getThreadCount() == 0)
		mImpl->mTaskPool->processTaskInThisThread(timer, timeOut);

	{	ScopeLock lock(mImpl->mMutex);
		mImpl->mResourceHolder.clear();
	}

	return loader;
}

void ResourceManager::addFactory(IFactory* factory)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);
	mImpl->addFactory(factory);
}

void ResourceManager::removeAllFactory()
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mMutex);
	mImpl->removeAllFactory();
}

TaskPool& ResourceManager::taskPool()
{
	MCD_ASSUME(mImpl != nullptr);
	return *mImpl->mTaskPool;
}

void IResourceLoader::PathKey::destroyThis()
{
	IResourceLoader* l = getOuterSafe();
	MCD_ASSUME(l);
	intrusivePtrRelease(l);
	l = nullptr;	// The loader may already deleted.
}

IResourceLoader::IResourceLoader()
	: Task(0), mPathKey("")//resource.fileId())
	, mLoadCount(0), mState(NotLoaded)
	, mResourceManager(nullptr)
	, mNeedEnqueu(true), mPendForCommit(false)
	, mOutstandingContinueCount(0)
{
}

void IResourceLoader::continueLoad()
{
	ScopeLock lock(mMutex);
	IResourceLoaderPtr holdThis(this);
	++mOutstandingContinueCount;
	if(mNeedEnqueu && mResourceManager->taskPool().enqueue(*this)) {
		mNeedEnqueu = false;
		// Increment the reference count to indicate the loader is shared by the thread
		intrusivePtrAddRef(this);
	}
}

void IResourceLoader::dependsOn(const IResourceLoaderPtr& loader)
{
	if(!loader)
		return;

	MCD_ASSERT(loader != this && "Should not depends on itself");

	ScopeLock lock(mMutex);
	mDepenseOn.push_back(loader.get());
	loader->mDepenseBy.push_back(this);
}

void IResourceLoader::releaseThis()
{
	{	ScopeLock lock(mMutex);
		mPathKey.removeThis();
//		if(!(mState & Stopped))
//			mState = Stopped;
	}

	// The following code may trigger delete, so don't include this into the scope lock.
	intrusivePtrRelease(static_cast<IntrusiveSharedWeakPtrTarget<AtomicInteger>*>(this));
}

void IResourceLoader::run(Thread& thread)
{
	IResourceLoaderPtr releaseWhenReturn(this, false);

	{	// Process as many load request as possible
		ScopeLock lock(mMutex);
		mNeedEnqueu = false;

		while(mOutstandingContinueCount > 0)
		{
			--mOutstandingContinueCount;

			// NOTE: A thread may wanna to quit because the user changed the thread pool's count,
			// so resubmit the task to the pool again and it will be run in other thread.
			// TODO: Return immediately if TaskPool wanna to quit.
			if(!thread.keepRun()) {
				if(mResourceManager && !mResourceManager->taskPool().getThreadCount() == 0)	{
					mNeedEnqueu = true;
					lock.unlockAndCancel();
					continueLoad();
				}
				return;
			}

			if(mState & IResourceLoader::Stopped) {
				mOutstandingContinueCount = 0;
				break;
			}

			_load(&mPathKey.getKey(), mArgs.c_str());
		}

		mNeedEnqueu = true;
		mPendForCommit = true;
	}

	{	// Pend for commit()
		MCD_ASSUME(mResourceManager && mResourceManager->mImpl);
		ResourceManager::Impl& impl = *mResourceManager->mImpl;
		impl.mEventQueue.pushBack(this);
	}
}

IResourceLoader::LoadingState IResourceLoader::_load(const Path* fileId, const char* args)
{
	MCD_ASSUME(args);

	if(fileId && !mIStream.get() && mResourceManager)
		mIStream = mResourceManager->mImpl->mFileSystem.openRead(*fileId);

	LoadingState state;
	{	ScopeUnlock unlock(mMutex);
		state = load(mIStream.get(), fileId, args);
	}

	mLoadCount++;
	// NOTE: Multiple load() may have been performed ,since load() may call continueLoad()
	// triggering thread task. Therefore we need to check mState before assiging it.
	mState = (mState & Stopped) ? mState : state;

	// Time to release the stream
	if(mState & Stopped)
		mIStream.reset();

	return mState;
}

const Path& IResourceLoader::fileId() const
{
	ScopeLock lock(mMutex);
	return mPathKey.getKey();
}

ResourcePtr IResourceLoader::resource() const
{
	ScopeLock lock(mMutex);
	return mResource.lock();
}

IResourceLoader::LoadingState IResourceLoader::loadingState() const
{
	ScopeLock lock(mMutex);
	return mState;
}

size_t IResourceLoader::loadCount() const
{
	ScopeLock lock(mMutex);
	return mLoadCount;
}

size_t IResourceLoader::dependencyParentCount() const
{
	ScopeLock lock(mMutex);
	return mDepenseBy.size();
}

// NOTE: We should return copy but not reference, since the std::vector may resize
// by another thread at anytime.
IResourceLoaderPtr IResourceLoader::getDependencyParent(size_t index) const
{
	ScopeLock lock(mMutex);
	if(index >= mDepenseBy.size())
		return nullptr;
	return mDepenseBy[index].lock();
}

size_t IResourceLoader::dependencyChildCount() const
{
	ScopeLock lock(mMutex);
	return mDepenseOn.size();
}

IResourceLoaderPtr IResourceLoader::getDependencyChild(size_t index) const
{
	ScopeLock lock(mMutex);
	if(index >= mDepenseOn.size())
		return nullptr;
	return mDepenseOn[index].lock();
}

}	// namespace MCD
