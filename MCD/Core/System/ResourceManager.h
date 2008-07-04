#ifndef __MCD_CORE_SYSTEM_RESOURCEMANAGER__
#define __MCD_CORE_SYSTEM_RESOURCEMANAGER__

#include "../ShareLib.h"
#include "IntrusivePtr.h"
#include "NonCopyable.h"

namespace MCD {

class IFileSystem;
class IResourceLoader;
class Path;
class Resource;
typedef IntrusivePtr<Resource> ResourcePtr;

/*!	Resource manager
 */
class MCD_NOVTABLE IResourceManager
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
	//! Factory for loading texture
	class TextureFactory : public ResourceManager::IFactory {
	public:
		sal_override ResourcePtr createResource(const Path& fileId) {
			return new Texture(fileId);
		}
		sal_override IResourceLoader* createLoader() {
			return new TextureLoader;
		}
	};	// TextureFactory

	// ...

	// Create a resource manager using the current directory
	ResourceManager manager(new MCD::RawFileSystem(L"./"));

	// Associate common texture format with TextureFactory 
	manager.associateFactory(L"png", new TextureFactory);
	manager.associateFactory(L"jpg", new TextureFactory);

	// ...

	// Some where in time you want to load a texture
	ResourcePtr resource = manager.load(
		L"logo.png",	// Path to the resource in the file system
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
	class MCD_NOVTABLE IFactory
	{
	public:
		virtual ResourcePtr createResource(const Path& path) = 0;
		virtual IResourceLoader* createLoader() = 0;
	};	// IFactory

	/*!	Construct ResourceManager and associate it with a file system.
		\note It will take ownership of \em fileSystem and delete it on ~ResourceManager()
			so make sure \em fileSystem is not created on the stack.
		\note By default 1 worker thread is used.
	 */
	explicit ResourceManager(IFileSystem& fileSystem);

	sal_override ~ResourceManager();

	/*!
		\note Blocking load will also generate events.
	 */
	sal_override ResourcePtr load(const Path& path, bool block=false, uint priority=0);

	/*! Event for notifying the loading status of a resource.
		\note The member \em resource and \em loader may be null to
			indicate this is an empty event.
	 */
	struct Event
	{
		ResourcePtr resource;
		IResourceLoader* loader;
	};	// Event

	Event popEvent();

	/*!	Associate resource factory with file extension.
		For example, "png" should be associated with a png texture factory.
		\param factory Put null to remove association.
			The lifetime of the factory will be controlled by ResourceManager.
	 */
	void associateFactory(sal_in_z const wchar_t* extension, sal_in_opt IFactory* factory);

	//! Get the underlaying TaskPool used by the ResourceManager.
	TaskPool& taskPool();

protected:
	class Impl;
	Impl* mImpl;
};	// ResourceManager

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCEMANAGER__
