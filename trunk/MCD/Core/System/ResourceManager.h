#ifndef __MCD_CORE_SYSTEM_RESOURCEMANAGER__
#define __MCD_CORE_SYSTEM_RESOURCEMANAGER__

#include "../ShareLib.h"
#include "IntrusivePtr.h"
#include "NonCopyable.h"
#include "Path.h"
#include "SharedPtr.h"
#include <list>

namespace MCD {

class IFileSystem;
class IResourceLoader;
class Path;
typedef IntrusivePtr<class Resource> ResourcePtr;

/*!	A class that represent a callback for the IResourceManager.
	As part of the loading process, we want some callback after certain resource(s)
	finished loading. What make this callback interesting is that you can register
	some dependency to it, so that the callback is only invoked after it's dependencies
	are resolved.
 */
class MCD_ABSTRACT_CLASS IResourceManagerCallback
{
public:
	virtual ~IResourceManagerCallback() {}

	/*!	Path is used as dependency instead of Resource to reduce ownership hell.
	 */
	virtual void addDependency(const Path& fileId) = 0;
};	// IResourceManagerCallback

/*!	A manager that handle the LOADING and CACHING of resources.
	\note
		This class only have a very few interface, because only those functions will be
		accessed by many different classes. Other functions that only accessed in a few
		places (mostly in the main loop) will left for the derive class to expose.
 */
class MCD_ABSTRACT_CLASS IResourceManager
{
public:
	virtual ~IResourceManager() {}

	/*!	Load a resource.
		The interface itself doesn't define how a resource is cached, what this function
		will be returned is totally up to the implementation.
		\param fileId Unique identifier for the resource to load.
		\param block Weather the function should block until the load process finish.
		\param priority The loading priority for background/progressive loading.
		\param extra arguments (encoded as a string) for loading the resource; similar to program arguments.
	 */
	virtual ResourcePtr load(const Path& fileId, bool block=false, uint priority=0, sal_in_z_opt const wchar_t* args=nullptr) = 0;

	/*!	Cache the given resource.
		The given resource is assumed to be fully loaded, no loading will be preformed
		on it, but a loading event (without a loader )will still get generated in order
		to make the dependency system work correctly.
		\return The previous cached resoure of the same fileId, if any.
	 */
	virtual ResourcePtr cache(const ResourcePtr& resource) = 0;

	/*!	Give up the control (un-cache it) over the resource with that fileId.
		Do nothing if that fileId is not already in the manager.
		Useful to reload a resource as a new instance.
	 */
	virtual void uncache(const Path& fileId) = 0;

	/*!	As part of the loading process, we want some callback after certain
		resource finished loading.
	 */
	virtual void addCallback(sal_in IResourceManagerCallback* callback) = 0;

	/*!	A place for the concret manager to do any necessary update.
		The return value is specific to the concret derived class.
	 */
	virtual int update() = 0;
};	// IResourceManager

class TaskPool;
class ResourceManagerCallback;

/*!	A default implementation of IResourceManager.
	Typical usage of ResourceManager:

	\code
	//! Factory for loading jpg texture
	class JpgLoaderFactory : public ResourceManager::IFactory {
	public:
		sal_override ResourcePtr createResource(const Path& fileId) {
			if(wstrCaseCmp(fileId.getExtension().c_str(), L"jpg") == 0)
				return new Texture(fileId);
			return nullptr;
		}
		sal_override IResourceLoader* createLoader() {
			return new JpgLoader;
		}
	};	// JpgLoaderFactory

	// ...

	// Create a resource manager using the current directory
	ResourceManager manager(new MCD::RawFileSystem(L"./"));

	// Associate a jpg loader to the manager
	manager.addFactory(new JpgLoaderFactory);

	// ...

	// Some where in time you want to load a texture
	ResourcePtr resource = manager.load(
		L"logo.jpg",	// Path to the resource in the file system
		false,			// True for blocking load; false for background load
		0				// The load priority (for background load)
	);

	// ...

	// In your main loop, you may want to get notification about the loading status
	while(true) {	// Poll for event
		ResourceManager::Event event = manager.popEvent();
		if(!event.loader)	// Break if there is no event in the queue
			break;

		IResourceLoader::LoadingState state = event.loader->getLoadingState();

		if(state | IResourceLoader::CanCommit) {	// Data is available
			event.loader->commit(*event.resource);
			// Do other things what ever you need
		}
		else if(state == Aborted)					// Something wrong during the load
			std::cout << "Error loading " << event.resource->fileId();
	}
	\endcode

	\note Since it use TaskPool to do the loading, all resource loader should
		be thread safe.
 */
class MCD_CORE_API ResourceManager : public IResourceManager, Noncopyable
{
public:
	//!	Extends this class to give ResourceManager knowledge about a particular type of resource.
	class MCD_ABSTRACT_CLASS IFactory
	{
	public:
		/*!	The overrides of this function should check the path to decide the filename/extension
			is of their interest or not. It should return a newly created resource if the file name
			match it's interest, return null otherwise.

			Example implementation for loading a jpg file:
			\code
			sal_override ResourcePtr createResource(const Path& file) {
				if(path.getExtension() == std::wstring(L"jpg"))
					return new Texture(file);
				return nullptr;
			}
			\endcode
		 */
		virtual ResourcePtr createResource(const Path& fileId, sal_in_z_opt const wchar_t* args) = 0;

		//! Overrided function should create and return a concrete resource loader.
		sal_notnull virtual IResourceLoader* createLoader() = 0;
	};	// IFactory

	/*!	Construct ResourceManager and associate it with a file system.
		\param takeFileSystemOwnership If true, ResourceManager will take ownership of \em fileSystem and delete it on ~ResourceManager()
		\note By default 1 worker thread is used.
	 */
	explicit ResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership=true);

	sal_override ~ResourceManager();

	/*!	Load a resource, see more on IResourceManager::load().
		If the requesting resource is already loaded by the manager before, the
		cached instance will be returned and no event generated.
		\note Blocking load will also generate events.
	 */
	sal_override ResourcePtr load(const Path& fileId, bool block=false, uint priority=0, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	By pass the cache and doing an explicitly reload.
		Return the original resource if reload is preformed, otherwise a new resource is
		returned just like calling load().
		\note This function is not part of the IResourceManager interface.
	 */
	ResourcePtr reload(const Path& fileId, bool block=false, uint priority=0, sal_in_z_opt const wchar_t* args=nullptr);

	/*!	Cache the given resource.
		The given resource is assumed to be fully loaded, no loading will be preformed
		on it, but a loading event (without a loader )will still get generated in order
		to make the dependency system work correctly.
		\return The previous cached resoure of the same fileId, if any.
	 */
	sal_override ResourcePtr cache(const ResourcePtr& resource);

	/*!	Give up the control (un-cache it) over the resource with that fileId.
		Do nothing if that fileId is not already in the manager.
		Useful to reload a resource as a new instance.
	 */
	sal_override void uncache(const Path& fileId);

	/*! Event for notifying the loading status of a resource.
		\note The member \em resource and \em loader may be null to
			indicate this is an empty event.
	 */
	struct Event
	{
		ResourcePtr resource;				//!< May be null
		SharedPtr<IResourceLoader> loader;	//!< May be null
	};	// Event

	/*!	Get any loading event from the event queue.
		If there is no event in the queue, a event with it's resource and loader
		equals to null will be returned.
	 */
	Event popEvent();

	/*!	Register a callback to be invoke later.
		Despite the invokcation order imposed by the dependency, the callback will be
		invoked as the order they were submitted to the ResourceManager.
		\param callback It's ownership will be taken by ResourceManager, therefore it
			must be created from heap. Do nothing if it is null.
		\note
			Do not invoke addDependency() after the callback is added to the manager,
			because the callback may be modified by the worker thread as well.
	 */
	sal_override void addCallback(sal_in IResourceManagerCallback* callback);

	sal_override int update() { return 0; }

	/*!	Resolve dependency and invoke the appropriate callback.
		\param event Supply with the event returned by popEvent()
	 */
	void doCallbacks(const Event& event);

	/*!	Adds a resource factory to the manager.
		\param factory Put null to remove association.
			The lifetime of the factory will be controlled by ResourceManager.
		\note A late added factory have a higher priority than other factory having the
			same file extension in the factory list. Therefore user can override any
			loader factory at any time.
	 */
	void addFactory(sal_in IFactory* factory);

	/*!	Remove all the factories associated.
		Will destroy them as well.
	 */
	void removeAllFactory();

	//! Get the underlaying TaskPool used by the ResourceManager.
	TaskPool& taskPool();

protected:
	class Impl;
	Impl* mImpl;
};	// ResourceManager

/*!	Callback for ResourceManager.
	Example of how to load a mesh after it's depending textures are loaded:
	\code
	// Extend your own callback class from ResourceManagerCallback
	class MyCallback : public ResourceManagerCallback {
	public:
		// Invoked after all depending resources are loaded.
		sal_override void doCallback() {
			*externalReference = manager->load(L"MyMesh.3ds");
		}

		ResourcePtr* externalReference;
		ResourceManager* manager;
	};	// MyCallback

	// ...

	ResouceManager manager;
	ResourcePtr mesh;

	// ...

	// Setting up the callback and add it to the resource manager
	MyCallback* callback = new MyCallback;
	callback->externalReference = &mesh;
	callback->manager = &manager;
	callback->addDependency(L"texture1.jpg");
	callback->addDependency(L"texture2.png");
	manager.addCallback(callback);

	// ...

	// Invoke doCallbacks()
	manager.doCallbacks(manager.popEvent());
	\endcode
 */
class MCD_CORE_API ResourceManagerCallback : public IResourceManagerCallback
{
	friend class ResourceManager;

public:
	/*!	Invoked every time the depending resource had FINISHED loaded (ie. loaded successfully
		or had error during loading but not partially loaded)
	 */
	virtual void doCallback() = 0;

	//! Do not invoke it concurrently.
	sal_override void addDependency(const Path& fileId);

	/*!	Setting the major dependency gives more information to ResourceManager such
		that only the ResourceManager::Event with that major dependence will trigger
		the callback.
		
		The main purpose of the function is to ensure the major dependency is commited
		before the callback is invoked. This commit-callback ordering problem occur
		when you load a resource synchronously and then add a callback before commit.
	 */
	void setMajorDependency(const Path& fileId);

	const Path& getMajorDependency() const;

protected:
	/*!	Return:
		<0		no dependency removed
		else	number of dependency remains
	 */
	int removeDependency(const Path& fileId);

	typedef std::list<Path> Paths;
	Paths mDependency;
	Path mMajorDependency;
};	// ResourceManagerCallback

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCEMANAGER__
