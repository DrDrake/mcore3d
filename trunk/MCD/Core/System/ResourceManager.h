#ifndef __MCD_CORE_SYSTEM_RESOURCEMANAGER__
#define __MCD_CORE_SYSTEM_RESOURCEMANAGER__

#include "../ShareLib.h"
#include "IntrusivePtr.h"
#include "NonCopyable.h"
#include "SharedPtr.h"

namespace MCD {

class IFileSystem;
class IResourceLoader;
class Path;
class Resource;
typedef IntrusivePtr<Resource> ResourcePtr;

/*!	Resource manager
 */
class MCD_ABSTRACT_CLASS IResourceManager
{
protected:
	virtual ~IResourceManager() {}

public:
	/*!	Load a resource.
		\param fileId Unique identifier for the resource to load.
		\param block Weather the function should block until the load process finish.
		\param priority The loading priority for background/progressive loading.
	 */
	virtual ResourcePtr load(const Path& fileId, bool block=false, uint priority=0) = 0;
};	// IResourceManager

class TaskPool;

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
		virtual ResourcePtr createResource(const Path& fileId) = 0;

		//! Overrided function should create and return a concrete resource loader.
		virtual IResourceLoader* createLoader() = 0;
	};	// IFactory

	/*!	Construct ResourceManager and associate it with a file system.
		\note It will take ownership of \em fileSystem and delete it on ~ResourceManager()
			so make sure \em fileSystem is not created on the stack.
		\note By default 1 worker thread is used.
	 */
	explicit ResourceManager(IFileSystem& fileSystem);

	sal_override ~ResourceManager();

	/*!	Load a resource, see more on IResourceManager::load().
		If the requesting resource is already loaded by the manager before, the
		cached instance will be returned and no event generated.
		\note Blocking load will also generate events.
	 */
	sal_override ResourcePtr load(const Path& fileId, bool block=false, uint priority=0);

	/*!	By pass the cache and doing an explicity reload.
		Return the original resource if reload is preformed, otherwise a new resource is
		returned just like calling load().
		\note This function is not part of the IResourceManager interface.
	 */
	ResourcePtr reload(const Path& fileId, bool block=false, uint priority=0);

	/*!	Give up the control (uncache it) over the resource with that fileId.
		Do nothing if that fileId is not already in the manager.
		Usefull to reload a resource as a new instance.
	 */
	void forget(const Path& fileId);

	/*! Event for notifying the loading status of a resource.
		\note The member \em resource and \em loader may be null to
			indicate this is an empty event.
	 */
	struct Event
	{
		ResourcePtr resource;
		SharedPtr<IResourceLoader> loader;
	};	// Event

	Event popEvent();

	/*!	Adds a resource factory to the manager.
		\param factory Put null to remove association.
			The lifetime of the factory will be controlled by ResourceManager.
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

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCEMANAGER__
