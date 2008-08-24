#include "Pch.h"
#include "ResourceManager.h"
#include "FileSystem.h"
#include "Macros.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "TaskPool.h"
#include "Utility.h"
#include <set>

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

	MapNode(Resource& resource, IResourceLoader& loader)
		:
		mPathKey(resource.fileId()),
		mResource(&resource),
		mLoader(loader)
	{
	}

	~MapNode()
	{
		delete &mLoader;
	}

	PathKey mPathKey;
	typedef WeakPtr<Resource> WeakResPtr;
	WeakResPtr mResource;
	IResourceLoader& mLoader;	// Keep a reference to the associated loader
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
			ScopeLock lock(mMutex);
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
			mLoader(loader),
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
					state = mLoader.load(mIStream.get());
				}

				Event event = { mResource, &mLoader };
				mEventQueue.pushBack(event);

				if(state & IResourceLoader::Stopped) {
					break;
				}
			}

			// Remember TaskPool::Task need to do cleanup after finished the job
			delete this;
		}

		ResourcePtr mResource;
		IResourceLoader& mLoader;
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
		mTaskPool.stop();

		delete &mFileSystem;
		removeAllFactory();
	}

	void blockingLoad(const Path& fileId, MapNode& node)
	{
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		do {
			if(node.mLoader.load(is.get()) & IResourceLoader::Stopped)
				break;
		} while(true);

		Event event = { node.mResource.get(), &(node.mLoader) };
		mEventQueue.pushBack(event);
	}

	void backgroundLoad(const Path& fileId, MapNode& node, uint priority)
	{
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		// Create the task to submit to the task pool
		Task* task = new Task(node.mResource.get(), node.mLoader, mEventQueue, is.get(), priority);
		is.release();	// We have transfered the ownership of istream to Task

		mTaskPool.enqueue(*task);
	}

	void addFactory(IFactory* factory)
	{
		mFactories.insert(factory);
	}

	void removeAllFactory()
	{
		// Delete all factories
		MCD_FOREACH(IFactory* factory, mFactories)
			delete factory;
		mFactories.clear();
	}

	ResourcePtr createResource(const Path& fileId, IResourceLoader*& loader)
	{
		ResourcePtr ret;
		// Loop for all factories to see which one will respondse to the fileId
		MCD_FOREACH(IFactory* factory, mFactories) {
			ret = factory->createResource(fileId);
			if(ret != nullptr) {
				loader = factory->createLoader();
				return ret;
			}
		}

		return nullptr;
	}

public:
	TaskPool mTaskPool;

	Map<MapNode::PathKey> mResourceMap;

	typedef std::set<IFactory*> Factories;
	Factories mFactories;

	EventQueue mEventQueue;

	IFileSystem& mFileSystem;
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

	{	// Find for existing resource
		MapNode* node = mImpl->mResourceMap.find(fileId)->getOuterSafe();

		if(node) {	// Cache hit!
			// Do clean up for dead resource node
			// It is preformed right here because a resource is shared,
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
	if(!resource)
		return nullptr;

	MCD_ASSUME(loader);

	MapNode* node = new MapNode(*resource, *loader);
	MCD_VERIFY(mImpl->mResourceMap.insertUnique(node->mPathKey));

	// Now we can begin the load operation
	if(block) {
		ScopeLock lock(mImpl->mEventQueue.mMutex);
		mImpl->blockingLoad(fileId, *node);
	}
	else
		mImpl->backgroundLoad(fileId, *node, priority);

	return resource;
}

ResourceManager::Event ResourceManager::popEvent()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mEventQueue.popFront();
}

void ResourceManager::addFactory(IFactory* factory)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->addFactory(factory);
}

void ResourceManager::removeAllFactory()
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->removeAllFactory();
}

TaskPool& ResourceManager::taskPool()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mTaskPool;
}

}	// namespace MCD
