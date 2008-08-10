#include "Pch.h"
#include "ResourceManager.h"
#include "FileSystem.h"
#include "Macros.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "TaskPool.h"
#include <map>

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
	// Use Path as the key
	struct PathKey : public MapBase<const Path&, const Path&>::Node<PathKey>
	{
		typedef MapBase<const Path&, const Path&>::Node<PathKey> Super;
		explicit PathKey(const Path& path) : Super(path) {}
		MCD_DECLAR_GET_OUTER_OBJ(MapNode, mPathKey);
		sal_override void destroyThis() throw() {
			delete getOuterSafe();
		}
	};

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
		mTaskPool.setThreadCount(0, true);

		delete &mFileSystem;

		// Delete all factories
		for(ExtensionMap::iterator i=mExtensionMap.begin(); i!=mExtensionMap.end(); ++i)
			delete i->second;
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

	void associateFactory(const wchar_t* extension, IFactory* factory)
	{
		ExtensionMap::iterator i = mExtensionMap.find(extension);

		if(i != mExtensionMap.end()) {
			delete i->second;	// delete any old entry first
			if(factory)
				i->second = factory;
			else
				mExtensionMap.erase(i);
		}
		else {
			if(factory)
				mExtensionMap.insert(ExtensionMap::value_type(extension, factory));
		}
	}

	IFactory* findFactory(const wchar_t* extension)
	{
		ExtensionMap::iterator i = mExtensionMap.find(extension);
		if(i != mExtensionMap.end())
			return i->second;
		return nullptr;
	}

public:
	TaskPool mTaskPool;

	Map<MapNode::PathKey> mResourceMap;

	typedef std::map<std::wstring, IFactory*> ExtensionMap;
	ExtensionMap mExtensionMap;

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
			MapNode* nextNode = node->mPathKey.next()->getOuterSafe();
			if(nextNode && !nextNode->mResource.get())
				delete nextNode;
			return node->mResource.get();
		}
	}

	// Create the resource and it's corresponding loader
	Path::string_type extension = fileId.getExtension();
	IFactory* factory = mImpl->findFactory(extension.c_str());

	if(!factory)
		return nullptr;

	ResourcePtr resource = factory->createResource(fileId);
	if(!resource)
		return nullptr;

	IResourceLoader* loader = factory->createLoader();
	if(!loader)
		return nullptr;

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

void ResourceManager::associateFactory(const wchar_t* extension, IFactory* factory)
{
	MCD_ASSUME(mImpl != nullptr);
	mImpl->associateFactory(extension, factory);
}

TaskPool& ResourceManager::taskPool()
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->mTaskPool;
}

}	// namespace MCD
