#include "Pch.h"
#include "ResourceManager.h"
#include "FileSystem.h"
#include "Log.h"
#include "Macros.h"
#include "MemoryProfiler.h"
#include "PtrVector.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "TaskPool.h"
#include "Utility.h"
#include <algorithm>	// for std::find

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <deque>
#ifdef MCD_VC
#	pragma warning(pop)
#endif

#define USE_MORE_PARALLEL_LOADING_SCHEDULE 1

namespace MCD {

struct MapNode
{
	// Use Path as the key, note that we should store a copy of the Path in PathKey rather than a reference,
	// because the resource can be destroyed at any time but the PathKey is destroyed at a later time.
	struct PathKey : public MapBase<Path, const Path&>::Node<PathKey>
	{
		typedef MapBase<Path, const Path&>::Node<PathKey> Super;
		explicit PathKey(const Path& path) : Super(path) {}
		MCD_DECLAR_GET_OUTER_OBJ(MapNode, mPathKey);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	};	// PathKey

	MapNode(Resource& resource)
		:
		mPathKey(resource.fileId()),
		mLoadingState(IResourceLoader::NotLoaded),
		mResource(&resource)
	{
	}

	PathKey mPathKey;
	IResourceLoader::LoadingState mLoadingState;	//!< As an information for resolving dependency
	typedef WeakPtr<Resource> WeakResPtr;
	WeakResPtr mResource;
};	// MapNode

class ResourceManager::Impl
{
	//! A thread safe event queue to make communication between the main thread and loader thread
	class EventQueue
	{
	public:
		typedef ResourceManager::Event Event;

		// This function is called inside the worker thread, and the mutex should already be locked
		void pushBack(const Event& event)
		{
			MCD_ASSERT(mMutex.isLocked());

			// Each instance of resource should appear in the event queue once.
			// Note: Linear search is enough since the event queue should keep consumed every frame
			MCD_FOREACH(const Event& e, mQueue) {
				if(e.resource == event.resource)
					return;
			}

			mQueue.push_back(event);
		}

		Event popFront()
		{
			MCD_ASSERT(mMutex.isLocked());
			Event ret = { nullptr, nullptr };

			if(!mQueue.empty()) {
				ret = mQueue.front();
				mQueue.pop_front();
			}

			return ret;
		}

		std::deque<Event> mQueue;
		Mutex mMutex;
	};	// EventQueue

	class Task : public MCD::TaskPool::Task
	{
	public:
		Task(MapNode& mapNode, const SharedPtr<IResourceLoader>& loader,
			EventQueue& eventQueue, std::istream* is, uint priority, TaskPool& taskPool, const wchar_t* args)
			:
			MCD::TaskPool::Task(priority),
			mMapNode(mapNode),
			mResource(mapNode.mResource.get()),
			mLoader(loader),
			mEventQueue(eventQueue),
			mIStream(is),
			mTaskPool(taskPool),
			mArgs(args == nullptr ? L"" : args)
		{
		}

		sal_override void run(Thread& thread) throw()
		{
			MemoryProfiler::Scope profiler("ResourceManager::Task::run");

			Mutex& mutex = mEventQueue.mMutex;
			ScopeLock lock(mutex);

			while(thread.keepRun()) {
				IResourceLoader::LoadingState state;
				{	ScopeUnlock unlock(mutex);
					state = mLoader->load(mIStream.get(), mResource ? &mResource->fileId() : nullptr, mArgs.c_str());
				}

				mMapNode.mLoadingState = state;
				Event event = { mResource, mLoader };
				mEventQueue.pushBack(event);

//				mSleep(1);

				if(state & IResourceLoader::Stopped) {
					break;
				}

				if(USE_MORE_PARALLEL_LOADING_SCHEDULE) {
					// Creat a new schedule that has a lower priority
					Task* task = new Task(mMapNode, mLoader, mEventQueue, mIStream.release(), priority()+1, mTaskPool, mArgs.c_str());
					mTaskPool.enqueue(*task);
					break;
				}
			}

			// Remember TaskPool::Task need to do cleanup after finished the job
			delete this;
		}

		MapNode& mMapNode;
		ResourcePtr mResource;				// Hold the life of the resource
		SharedPtr<IResourceLoader> mLoader;	// Hold the life of the IResourceLoader
		EventQueue& mEventQueue;
		std::auto_ptr<std::istream> mIStream;	// Keep the life of the stream align with the Task
		TaskPool& mTaskPool;
		std::wstring mArgs;
	};	// Task

public:
	Impl(IFileSystem& fileSystem) : mFileSystem(fileSystem)
	{
		mTaskPool.setThreadCount(1);
	}

	~Impl()
	{
		// Stop the task pool first
		// Tasks may be invoked in this thread context and so they
		// may try to acquire mEventQueue.mMutex. Acquiring the mutex
		// before calling task pool stop will result a dead lock.
		mTaskPool.stop();

		{	ScopeLock lock(mEventQueue.mMutex);
			delete &mFileSystem;
			removeAllFactory();
		}
	}

	MapNode* findMapNode(const Path& fileId)
	{
		return mResourceMap.find(fileId)->getOuterSafe();
	}

	void blockingLoad(const Path& fileId, const wchar_t* args, MapNode& node, IResourceLoader& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		do {
			ScopeUnlock unlock(mEventQueue.mMutex);
			if(loader.load(is.get(), &fileId, args) & IResourceLoader::Stopped)
				break;
		} while(true);

		node.mLoadingState = loader.getLoadingState();
		Event event = { node.mResource.get(), &loader };
		mEventQueue.pushBack(event);
	}

	void backgroundLoad(const Path& fileId, const wchar_t* args, MapNode& node, IResourceLoader& loader, uint priority)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		// Create the task to submit to the task pool
		Task* task = new Task(node, &loader, mEventQueue, is.get(), priority, mTaskPool, args);
		is.release();	// We have transfered the ownership of istream to Task

		mTaskPool.enqueue(*task);
	}

	void addFactory(IFactory* factory)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		mFactories.push_back(factory);
	}

	void removeAllFactory()
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		mFactories.clear();
	}

	ResourcePtr createResource(const Path& fileId, const wchar_t* args, IResourceLoader*& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		ResourcePtr ret;
		// Loop for all factories to see which one will response to the fileId
		for(Factories::iterator i=mFactories.begin(); i!=mFactories.end(); ++i) {
			ret = i->createResource(fileId, args);
			if(ret != nullptr) {
				loader = i->createLoader();
				return ret;
			}
		}

		return nullptr;
	}

public:
	TaskPool mTaskPool;

	Map<MapNode::PathKey> mResourceMap;

	typedef ptr_vector<IFactory> Factories;
	Factories mFactories;

	EventQueue mEventQueue;

	IFileSystem& mFileSystem;

	typedef ptr_vector<ResourceManagerCallback> Callbacks;
	Callbacks mCallbacks;
};	// Impl

ResourceManager::ResourceManager(IFileSystem& fileSystem)
{
	mImpl = new Impl(fileSystem);
}

ResourceManager::~ResourceManager()
{
	delete mImpl;
}

ResourcePtr ResourceManager::load(const Path& fileId, bool block, uint priority, const wchar_t* args)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	{	// Find for existing resource
		MapNode* node = mImpl->findMapNode(fileId);

		while(node) {	// Cache hit!
			// But unfortunately, the resource is already deleted
			if(!node->mResource) {
				delete node;
				break;
			}

			// Do clean up for dead resource node
			// It is performed right here because a resource is shared,
			// and we have no idea when the resource will be destroyed other
			// than poll for it's weak pointer periodically
			// To some extend, this is a garbage collector!
			MapNode* nextNode = node->mPathKey.next()->getOuterSafe();
			if(nextNode && !nextNode->mResource)
				delete nextNode;
			return node->mResource.get();
		}
	}

	IResourceLoader* loader = nullptr;
	ResourcePtr resource = mImpl->createResource(fileId, args, loader);
	if(!resource || !loader) {
		Log::format(Log::Warn, L"No loader for \"%s\" can be found", fileId.getString().c_str());
		delete loader;
		return nullptr;
	}

	MapNode* node = new MapNode(*resource);
	MCD_VERIFY(mImpl->mResourceMap.insertUnique(node->mPathKey));

	// Now we can begin the load operation
	if(block)
		mImpl->blockingLoad(fileId, args, *node, *loader);
	else
		mImpl->backgroundLoad(fileId, args, *node, *loader, priority);

	return resource;
}

ResourcePtr ResourceManager::reload(const Path& fileId, bool block, uint priority, const wchar_t* args)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find for existing resource
	MapNode* node = mImpl->findMapNode(fileId);

	// The resource is not found
	if(!node || !node->mResource) {
		lock.m.unlock();
		lock.cancel();
		return load(fileId, block, priority);
	}

	// We are only interested in the loader, the returned resource pointer is simple ignored.
	IResourceLoader* loader = nullptr;
	if(!mImpl->createResource(fileId, args, loader) || !loader) {
		delete loader;
		return nullptr;
	}

	// Now we can begin the load operation
	if(block)
		mImpl->blockingLoad(fileId, args, *node, *loader);
	else
		mImpl->backgroundLoad(fileId, args, *node, *loader, priority);

	return node->mResource.get();
}

ResourcePtr ResourceManager::cache(const ResourcePtr& resource)
{
	if(!resource)
		return resource;

	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	ResourcePtr ret;
	MapNode* node;

	// Find for existing resource
	if((node = mImpl->findMapNode(resource->fileId())) != nullptr) {
		ret = node->mResource.get();
		delete node;
		node = nullptr;
	}

	node = new MapNode(*resource);
	node->mLoadingState = IResourceLoader::Loaded;	// We assume the suppling resource is already loaded
	MCD_VERIFY(mImpl->mResourceMap.insertUnique(node->mPathKey));

	// Generate a finished loading event
	Event event = { resource.get(), nullptr };
	mImpl->mEventQueue.pushBack(event);

	return ret;
}

void ResourceManager::uncache(const Path& fileId)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find and remove the existing resource from the manager
	delete mImpl->findMapNode(fileId);
}

ResourceManager::Event ResourceManager::popEvent()
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);
	return mImpl->mEventQueue.popFront();
}

void ResourceManager::addCallback(IResourceManagerCallback* callback)
{
	ResourceManagerCallback* cb = dynamic_cast<ResourceManagerCallback*>(callback);
	if(!cb)
		return;

	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);	// Mutex against doCallbacks()

	// Resolve those already loaded dependency
	for(std::list<Path>::iterator i=cb->mDependency.begin(); i!=cb->mDependency.end();)
	{
		MapNode* node = nullptr;
		if((node = mImpl->findMapNode(*i)) != nullptr &&	// Conditions for already loaded resource
			node->mLoadingState & IResourceLoader::Stopped)
		{
			i = cb->mDependency.erase(i);
		}
		else
			++i;
	}

	mImpl->mCallbacks.push_back(cb);
}

void ResourceManager::doCallbacks(const Event& event)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);	// Mutex against addCallback()

	Impl::Callbacks& callbacks = mImpl->mCallbacks;
	if(event.resource)
	{
		// We ignore partial loading event
		if(event.loader && !(event.loader->getLoadingState() & IResourceLoader::Stopped))
			return;

		for(Impl::Callbacks::iterator i=callbacks.begin(); i!=callbacks.end();) {
			if(i->removeDependency(event.resource->fileId()) == 0) {
				i->doCallback();
				i = callbacks.erase(i, true);	// Be careful of erasing element during iteration
			}
			else
				++i;
		}
	} else
	{
		for(Impl::Callbacks::iterator i=callbacks.begin(); i!=callbacks.end();) {
			if(i->mDependency.empty()) {
				i->doCallback();
				i = callbacks.erase(i, true);	// Be careful of erasing element during iteration
			}
			else
				++i;
		}
	}
}

void ResourceManager::addFactory(IFactory* factory)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);
	mImpl->addFactory(factory);
}

void ResourceManager::removeAllFactory()
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);
	mImpl->removeAllFactory();
}

TaskPool& ResourceManager::taskPool()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mTaskPool;
}

void ResourceManagerCallback::addDependency(const Path& fileId)
{
	mDependency.push_back(fileId);
}

int ResourceManagerCallback::removeDependency(const Path& fileId)
{
	if(mDependency.empty())
		return 0;

	Paths::iterator i=std::find(mDependency.begin(), mDependency.end(), fileId);
	if(i == mDependency.end())
		return -1;
	mDependency.erase(i);
	return mDependency.size();
}

}	// namespace MCD
