#include "Pch.h"
#include "ResourceManager.h"
#include "FileSystem.h"
#include "Macros.h"
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
		mResource(&resource)
	{
	}

	PathKey mPathKey;
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
		Task(const ResourcePtr& resource, IResourceLoader& loader, EventQueue& eventQueue, std::istream* is, uint priority)
			:
			MCD::TaskPool::Task(priority),
			mResource(resource),
			mLoader(&loader),
			mEventQueue(eventQueue),
			mIStream(is)
		{
		}

		sal_override void run(Thread& thread) throw()
		{
			Mutex& mutex = mEventQueue.mMutex;
			ScopeLock lock(mutex);

			while(thread.keepRun()) {
				IResourceLoader::LoadingState state;
				{	ScopeUnlock unlock(mutex);
					state = mLoader->load(mIStream.get(), mResource ? &mResource->fileId() : nullptr);
				}

				Event event = { mResource, mLoader };
				mEventQueue.pushBack(event);

				if(state & IResourceLoader::Stopped) {
					break;
				}
			}

			// Remember TaskPool::Task need to do cleanup after finished the job
			delete this;
		}

		ResourcePtr mResource;
		SharedPtr<IResourceLoader> mLoader;	// Hold the life of the IResourceLoader
		EventQueue& mEventQueue;
		std::auto_ptr<std::istream> mIStream;	// Keep the life of the stream align with the Task
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
		// may try to acquire mEventQueue.mMutex. Acquring the mutex
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

	void blockingLoad(const Path& fileId, MapNode& node, IResourceLoader& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		do {
			ScopeUnlock unlock(mEventQueue.mMutex);
			if(loader.load(is.get(), &fileId) & IResourceLoader::Stopped)
				break;
		} while(true);

		Event event = { node.mResource.get(), &loader };
		mEventQueue.pushBack(event);
	}

	void backgroundLoad(const Path& fileId, MapNode& node, IResourceLoader& loader, uint priority)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		// Create the task to submit to the task pool
		Task* task = new Task(node.mResource.get(), loader, mEventQueue, is.get(), priority);
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

	ResourcePtr createResource(const Path& fileId, IResourceLoader*& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		ResourcePtr ret;
		// Loop for all factories to see which one will respondse to the fileId
		for(Factories::iterator i=mFactories.begin(); i!=mFactories.end(); ++i) {
			ret = i->createResource(fileId);
			if(ret != nullptr) {
				loader = i->createLoader();
				return ret;
			}
		}

		return nullptr;
	}

	void updateDependency(const Event& e)
	{
		MCD_ASSERT(e.resource);

		// We ignore partial loading event
		if(!(e.loader->getLoadingState() & IResourceLoader::Stopped))
			return;

		for(Callbacks::iterator i=mCallbacks.begin(); i!=mCallbacks.end();) {
			int numDependencyLeft = i->removeDependency(e.resource->fileId());
			if(numDependencyLeft >= 0)
				i->doCallback(e, size_t(numDependencyLeft));

			if(numDependencyLeft == 0)
				i = mCallbacks.erase(i, true);
			else
				++i;
		}
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

ResourcePtr ResourceManager::load(const Path& fileId, bool block, uint priority)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	{	// Find for existing resource
		MapNode* node = mImpl->findMapNode(fileId);

		while(node) {	// Cache hit!
			// But unfortunately, the resource is already deleted
			if(!node->mResource.get()) {
				delete node;
				break;
			}

			// Do clean up for dead resource node
			// It is performed right here because a resource is shared,
			// and we have no idea when the resource will be destroyed other
			// than poll for it's weak pointer periodically
			// To some extend, this is a garbage collector!
			MapNode* nextNode = node->mPathKey.next()->getOuterSafe();
			if(nextNode && !nextNode->mResource.get())
				delete nextNode;
			return node->mResource.get();
		}
	}

	IResourceLoader* loader = nullptr;
	ResourcePtr resource = mImpl->createResource(fileId, loader);
	if(!resource || !loader) {
		delete loader;
		return nullptr;
	}

	MapNode* node = new MapNode(*resource);
	MCD_VERIFY(mImpl->mResourceMap.insertUnique(node->mPathKey));

	// Now we can begin the load operation
	if(block)
		mImpl->blockingLoad(fileId, *node, *loader);
	else
		mImpl->backgroundLoad(fileId, *node, *loader, priority);

	return resource;
}

ResourcePtr ResourceManager::reload(const Path& fileId, bool block, uint priority)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find for existing resource
	MapNode* node = mImpl->findMapNode(fileId);

	// The resource is not found
	if(!node || !node->mResource.get()) {
		mImpl->mEventQueue.mMutex.unlock();
		lock.cancel();
		return load(fileId, block, priority);
	}

	// We are only interested in the loader, the returned resource pointer is simple ignored.
	IResourceLoader* loader = nullptr;
	if(!mImpl->createResource(fileId, loader) || !loader) {
		delete loader;
		return nullptr;
	}

	// Now we can begin the load operation
	if(block)
		mImpl->blockingLoad(fileId, *node, *loader);
	else
		mImpl->backgroundLoad(fileId, *node, *loader, priority);

	return node->mResource.get();
}

void ResourceManager::forget(const Path& fileId)
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
	MCD_ASSUME(mImpl != nullptr);
	if(!callback)
		return;

	ScopeLock lock(mImpl->mEventQueue.mMutex);	// Mutex against doCallbacks()
	mImpl->mCallbacks.push_back(dynamic_cast<ResourceManagerCallback*>(callback));
}

void ResourceManager::doCallbacks(const Event& event)
{
	MCD_ASSUME(mImpl != nullptr);
	if(!event.resource)
		return;

	ScopeLock lock(mImpl->mEventQueue.mMutex);	// Mutex against addCallback()
	mImpl->updateDependency(event);
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
	Paths::iterator i=std::find(mDependency.begin(), mDependency.end(), fileId);
	if(i == mDependency.end())
		return -1;
	mDependency.erase(i);
	return mDependency.size();
}

}	// namespace MCD
