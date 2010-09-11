#ifndef __MCD_CORE_SYSTEM_RESOURCEMANAGER__
#define __MCD_CORE_SYSTEM_RESOURCEMANAGER__

#include "NonCopyable.h"
#include "Path.h"
#include "SharedPtr.h"
#include "WeakPtr.h"
#include <list>

namespace MCD {

class IFileSystem;
class TaskPool;
class Timer;
typedef IntrusivePtr<class Resource> ResourcePtr;
typedef IntrusivePtr<class IResourceLoader> IResourceLoaderPtr;

/*!	A manager that handle the LOADING and CACHING of resources.
	Typical usage of ResourceManager:

	\code
	//! Factory for loading jpg texture
	class JpgLoaderFactory : public ResourceManager::IFactory {
	public:
		sal_override IResourceLoaderPtr createResource(const Path& fileId) {
			if(wstrCaseCmp(fileId.getExtension().c_str(), "jpg") == 0)
				return new Texture(fileId);
			return nullptr;
		}
		sal_override IResourceLoader* createLoader() {
			return new JpgLoader;
		}
	};	// JpgLoaderFactory

	// ...

	// Create a resource manager using the current directory
	ResourceManager manager(new MCD::RawFileSystem("./"));

	// Associate a jpg loader to the manager
	manager.addFactory(new JpgLoaderFactory);

	// ...

	// Some where in time you want to load a texture
	ResourcePtr resource = manager.load(
		"logo.jpg",	// Path to the resource in the file system
		1,			// >0 for blocking load; 0 for background load
		0			// The load priority (for background load)
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
class MCD_CORE_API ResourceManager : private Noncopyable
{
public:
	//!	Extends this class to give ResourceManager knowledge about a particular type of resource.
	class MCD_ABSTRACT_CLASS IFactory
	{
	public:
		virtual ~IFactory() {}

		/*!	The overrides of this function should check the path to decide the filename/extension
			is of their interest or not. It should return a newly created resource if the file name
			match it's interest, return null otherwise.

			Example implementation for loading a jpg file:
			\code
			sal_override IResourceLoaderPtr createResource(const Path& file) {
				if(path.getExtension() == std::string("jpg"))
					return new Texture(file);
				return nullptr;
			}
			\endcode
		 */
		sal_maybenull virtual ResourcePtr createResource(const Path& fileId, sal_in_z_opt const char* args) = 0;

		//! Overrided function should create and return a concrete resource loader.
		sal_notnull virtual IResourceLoaderPtr createLoader() = 0;
	};	// IFactory

	/*!	Construct ResourceManager and associate it with a file system.
		\param takeFileSystemOwnership If true, ResourceManager will take ownership of \em fileSystem and delete it on ~ResourceManager()
		\note One thread is created internally.
	 */
	explicit ResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership=true);

	/*!	Constructor that take an external TaskPool for job processing.
		\note The taskPool should has a longer life time than the ResourceManager, and make sure
			the taskPool is in the stopped state before ResourceManager get destroyed.
	 */
	ResourceManager(IFileSystem& fileSystem, TaskPool& taskPool, bool takeFileSystemOwnership=true);

	~ResourceManager();

// Operations
	/*!	Load a resource.
		It should be the most frequently used function of the ResourceManager.
		If the requesting resource is already loaded by the manager before, the
		cached instance will be returned and no event generated.
		\note If the load failed, a commit() will still issued.
		\param fileId Unique identifier for the resource to load.
		\param block Weather the function should block until the load process finish.
		\param priority The loading priority for background/progressive loading.
		\param args Extra arguments (encoded as name-value paired strings) for loading the resource; similar to program arguments.
		\param loader An optional parameter to retrieve the IResourceLoader.
	 */
	ResourcePtr load(const Path& fileId, int blockIteration=-1, int priority=0, sal_in_z_opt const char* args=nullptr);

	/*!	By pass the cache and doing an explicitly reload.
		Return the original resource if reload is preformed, otherwise a new resource is
		returned just like calling load().
	 */
	ResourcePtr reload(const Path& fileId, int blockIteration=-1, int priority=0, sal_in_z_opt const char* args=nullptr);

	/// If you need to use IResourceLoader::continueLoad(), you need to get this.
	/// Return null if the manager haven't cache about the specified resource.
	sal_maybenull IResourceLoaderPtr getLoader(const Path& fileId);

	/*!	Cache the given resource.
		The given resource is assumed to be fully loaded, no loading will be preformed
		on it, but a loading event (without a loader )will still get generated in order
		to make the dependency system work correctly.
		\return The previous cached resoure of the same fileId, if any.
	 */
	ResourcePtr cache(const ResourcePtr& resource);

	/*!	Give up the control (un-cache it) over the resource with that fileId.
		Do nothing if that fileId is not already in the manager.
		Useful to reload a resource as a new instance.
		\return The previous cached resource, if any.
	 */
	ResourcePtr uncache(const Path& fileId);

	/*!
		\note Caching is NOT used or altered in customLoad().
	 */
	void customLoad(const ResourcePtr& resource, const IResourceLoaderPtr& loader, int blockIteration=-1, int priority=0, sal_in_z_opt const char* args=nullptr);

	/*!	Get any loading event from the event queue.
		Return null if no event happens, or time-out expired.
		\param performLoad Wether this function will process some of the loading task.
		\note The commit() operation is done in this function.
	 */
	sal_maybenull IResourceLoaderPtr popEvent(sal_maybenull Timer* timer=nullptr, float timeOut=0, bool performLoad=false);

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

// Attributes
	//! Get the underlaying TaskPool used by the ResourceManager.
	TaskPool& taskPool();

protected:
	friend class IResourceLoader;
	class Impl;
	Impl* mImpl;
};	// ResourceManager

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCEMANAGER__
