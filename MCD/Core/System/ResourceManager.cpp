#include "Pch.h"
#include "ResourceManager.h"
#include "Deque.h"
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

namespace MCD {

namespace {

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
		mResource(&resource), mIsUncached(false)
	{
	}

	PathKey mPathKey;
	IResourceLoader::LoadingState mLoadingState;	//!< As an information for resolving dependency
	ResourceWeakPtr mResource;

	/*!	Due to the importance of MapNode, we cannot simply delete a MapNode or set mResource to null
		to preform a resource uncache operation.
	 */
	bool mIsUncached;
};	// MapNode

}	// namespace

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

public:
	class Task : public MCD::TaskPool::Task, public IPartialLoadContext
	{
	public:
		Task(ResourceManager& manager, MapNode& mapNode, const IResourceLoaderPtr& loader,
			EventQueue& eventQueue, std::istream* is, uint priority, const char* args)
			:
			MCD::TaskPool::Task(priority),
			mResourceManager(manager),
			mMapNode(mapNode),
			mResource(mapNode.mResource.get()),
			mLoader(loader),
			mEventQueue(eventQueue),
			mIStream(is),
			mArgs(args == nullptr ? "" : args)
		{
		}

		sal_override void run(Thread& thread) throw()
		{
			MemoryProfiler::Scope profiler("ResourceManager::Task::run");

			while(thread.keepRun())
			{
				IResourceLoader::LoadingState state;
				state = mLoader->load(mIStream.get(), mResource ? &mResource->fileId() : nullptr, mArgs.c_str());

				{	ScopeLock lock(mEventQueue.mMutex);
					mMapNode.mLoadingState = state;
					Event event = { mResource, mLoader };
					mEventQueue.pushBack(event);
				}

//				mSleep(1);

				if(state & IResourceLoader::Stopped)
					break;

				Task* task = new Task(mResourceManager, mMapNode, mLoader, mEventQueue, mIStream.release(), priority(), mArgs.c_str());

				// The onPartialLoaded() callback will decide when to continue the partial load
				mLoader->onPartialLoaded(*task, priority(), mArgs.c_str());
				break;
			}

			// Remember TaskPool::Task need to do cleanup after finished the job
			delete this;
		}

		sal_override void continueLoad(uint priority, const char* args)
		{
			// NOTE: There is no need to lock, since mTaskPool's operation is already thread safe
			setPriority(priority);
			mArgs = args ? args : "";
			MCD_VERIFY(mResourceManager.taskPool().enqueue(*this));
		}

		ResourceManager& mResourceManager;
		MapNode& mMapNode;
		ResourcePtr mResource;				// Hold the life of the resource
		IResourceLoaderPtr mLoader;			// Hold the life of the IResourceLoader
		EventQueue& mEventQueue;
		std::auto_ptr<std::istream> mIStream;	// Keep the life of the stream align with the Task
		std::string mArgs;
	};	// Task

public:
	Impl(TaskPool* externalTaskPool, ResourceManager& manager, IFileSystem& fileSystem, bool takeFileSystemOwnership)
		: mTaskPool(externalTaskPool)
		, mResourceManager(manager)
		, mFileSystem(fileSystem)
		, mTakeFileSystemOwnership(takeFileSystemOwnership)
	{
		mIsExternalTaskPool = externalTaskPool != nullptr;
		if(!externalTaskPool)
			mTaskPool.reset(new TaskPool);

		mTaskPool->setThreadCount(1);
	}

	~Impl()
	{
		// Stop the task pool first
		// Tasks may be invoked in this thread context and so they
		// may try to acquire mEventQueue.mMutex. Acquiring the mutex
		// before calling task pool stop will result a dead lock.
		mTaskPool->stop();

		{	ScopeLock lock(mEventQueue.mMutex);
			if(mTakeFileSystemOwnership)
				delete &mFileSystem;
			removeAllFactory();
		}

		if(mIsExternalTaskPool)
			mTaskPool.release();
	}

	MapNode* findMapNode(const Path& fileId)
	{
		return mResourceMap.find(fileId)->getOuterSafe();
	}

	void blockingLoad(const Path& fileId, const char* args, MapNode& node, const IResourceLoaderPtr& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		do {
			ScopeUnlock unlock(mEventQueue.mMutex);
			if(loader->load(is.get(), &fileId, args) & IResourceLoader::Stopped)
				break;
		} while(true);

		node.mLoadingState = loader->getLoadingState();
		Event event = { node.mResource.get(), loader };
		mEventQueue.pushBack(event);
	}

	void backgroundLoad(const Path& fileId, const char* args, MapNode& node, const IResourceLoaderPtr& loader, bool firstPartialBlock, uint priority)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		std::auto_ptr<std::istream> is(mFileSystem.openRead(fileId));

		// Create the task to submit to the task pool
		Task* task = new Task(mResourceManager, node, loader, mEventQueue, is.get(), priority, args);
		is.release();	// We have transfered the ownership of istream to Task

		// Prevent lock hierarchy.
		ScopeUnlock unlock(mEventQueue.mMutex);

		if(firstPartialBlock) {
			// Note that the variable "is" is already release and equals to null, use task->mIStream instead.
			if(loader->load(task->mIStream.get(), &fileId, args))
				loader->onPartialLoaded(*task, priority, args);
		} else {
			MCD_VERIFY(mTaskPool->enqueue(*task));
		}
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

	ResourcePtr createResource(const Path& fileId, const char* args, IResourceLoaderPtr& loader)
	{
		MCD_ASSERT(mEventQueue.mMutex.isLocked());
		ResourcePtr ret;
		// Loop for all factories to see which one will response to the fileId
		// NOTE: We loop the factories in reverse order, so that user can override a new type of 
		// loader factory by inserting a new one.
		for(Factories::reverse_iterator i=mFactories.rbegin(); i!=mFactories.rend(); ++i) {
			ret = i->createResource(fileId, args);
			if(ret != nullptr) {
				loader = i->createLoader();
				return ret;
			}
		}

		return nullptr;
	}

public:
	std::auto_ptr<TaskPool> mTaskPool;
	bool mIsExternalTaskPool;

	Map<MapNode::PathKey> mResourceMap;

	typedef ptr_vector<IFactory> Factories;
	Factories mFactories;

	EventQueue mEventQueue;

	ResourceManager& mResourceManager;
	IFileSystem& mFileSystem;
	bool mTakeFileSystemOwnership;

	typedef ptr_vector<ResourceManagerCallback> Callbacks;
	Callbacks mCallbacks;
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

ResourcePtr ResourceManager::load(const Path& fileId, BlockingMode blockingMode, uint priority, const char* args, IResourceLoaderPtr* _loader)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find for existing resource (Cache hit!)
	MapNode* node = mImpl->findMapNode(fileId);
	if(node)
	{
		// NOTE: Prevent the resource being deleted in another thread.
		ScopeLock lock2(node->mResource.destructionMutex());

		ResourcePtr p = node->mResource.get();

		if(node->mResource)	// Weak pointer not null, the resource is still alive
		{
			// Do clean up for dead resource node
			// It is performed right here because a resource is shared,
			// and we have no idea when the resource will be destroyed other
			// than poll for it's weak pointer periodically
			// To some extend, this is a garbage collector!
			MapNode* nextNode = node->mPathKey.next()->getOuterSafe();
			if(nextNode && !nextNode->mResource)
				delete nextNode;

			if(!node->mIsUncached)
				return p;
		}
		else {	// Unfortunately, the resource is already deleted
			lock2.mutex().unlock();
			lock2.cancel();
			delete node;
			node = nullptr;
		}
	}

	IResourceLoaderPtr loader = nullptr;
	ResourcePtr resource = mImpl->createResource(fileId, args, loader);

	if(_loader)
		*_loader = loader;

	if(!resource || !loader) {
		Log::format(Log::Warn, "No loader for \"%s\" can be found", fileId.getString().c_str());

		if(_loader)
			*_loader = nullptr;

		return nullptr;
	}

	if(!node) {
		node = new MapNode(*resource);
		MCD_VERIFY(mImpl->mResourceMap.insertUnique(node->mPathKey));
	} else {
		node->mIsUncached = false;
		node->mResource = resource.get();
		node->mLoadingState = IResourceLoader::NotLoaded;
	}

	// Now we can begin the load operation
	if(blockingMode == Block)
		mImpl->blockingLoad(fileId, args, *node, loader);
	else
		mImpl->backgroundLoad(fileId, args, *node, loader, blockingMode==FirstPartialBlock, priority);

	return resource;
}

std::pair<IResourceLoaderPtr, ResourcePtr> ResourceManager::customLoad(const Path& fileId, const char* args)
{
	MCD_ASSUME(mImpl != nullptr);
	IResourceLoaderPtr loader = nullptr;

	ResourcePtr resource;
	{	ScopeLock lock(mImpl->mEventQueue.mMutex);
		resource = mImpl->createResource(fileId, args, loader);
	}

	if(!resource || !loader) {
		Log::format(Log::Warn, "No loader for \"%s\" can be found", fileId.getString().c_str());
		return std::make_pair(IResourceLoaderPtr(nullptr), (Resource*)nullptr);
	}

	return std::make_pair(loader, resource);
}

ResourcePtr ResourceManager::reload(const Path& fileId, BlockingMode blockingMode, uint priority, const char* args)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find for existing resource
	MapNode* node = mImpl->findMapNode(fileId);

	// The resource is not found
	if(!node || !node->mResource || node->mIsUncached) {
		lock.mutex().unlock();
		lock.cancel();
		return load(fileId, blockingMode, priority);
	}

	// We are only interested in the loader, the returned resource pointer is simple ignored.
	IResourceLoaderPtr loader = nullptr;
	if(!mImpl->createResource(fileId, args, loader) || !loader)
		return nullptr;

	// Now we can begin the load operation
	if(blockingMode == Block)
		mImpl->blockingLoad(fileId, args, *node, loader);
	else
		mImpl->backgroundLoad(fileId, args, *node, loader, blockingMode==FirstPartialBlock, priority);

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

	if(resource->fileId().getString().empty()) {
		MCD_ASSERT(false && "It's meaningless to cache a resource without a name");
		return nullptr;
	}

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

ResourcePtr ResourceManager::uncache(const Path& fileId)
{
	MCD_ASSUME(mImpl != nullptr);
	ScopeLock lock(mImpl->mEventQueue.mMutex);

	// Find and remove the existing resource linkage from the manager
	MapNode* mapNode = mImpl->findMapNode(fileId);
	if(!mapNode)
		return nullptr;

	mapNode->mIsUncached = true;
	return mapNode->mResource.get();
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
			// See if major dependency constrain applied
			if(!i->mMajorDependency.getString().empty() && i->mMajorDependency != event.resource->fileId()) {
				++i;
				continue;
			}

			if(i->removeDependency(event.resource->fileId()) == 0) {
				i->doCallback();
				i = callbacks.erase(i, true);	// Be careful of erasing element during iteration
			}
			else
				++i;
		}
	}
	else	// It's an empty event
	{
		for(Impl::Callbacks::iterator i=callbacks.begin(); i!=callbacks.end();) {
			// NOTE: We need not to check for major dependency because:
			// -Major dependency not loaded then i->mDependency will not be empty;
			// -Major dependency loaded but not commited then ResourceManager event list will not be empty
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
	return *mImpl->mTaskPool;
}

void ResourceManagerCallback::addDependency(const Path& fileId)
{
	mDependency.push_back(fileId);
}

void ResourceManagerCallback::setMajorDependency(const Path& fileId)
{
	mMajorDependency = fileId;
	addDependency(fileId);
}

const Path& ResourceManagerCallback::getMajorDependency() const {
	return mMajorDependency;
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

void IResourceLoader::onPartialLoaded(IPartialLoadContext& context, uint priority, const char* args)
{
	context.continueLoad(priority, args);
}

}	// namespace MCD
