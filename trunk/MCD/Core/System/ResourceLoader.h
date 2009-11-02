#ifndef __MCD_CORE_SYSTEM_RESOURCELOADER__
#define __MCD_CORE_SYSTEM_RESOURCELOADER__

#include "Platform.h"
#include "../ShareLib.h"
#include <iosfwd>

namespace MCD {

class IResourceManager;
class Resource;
class Path;

/*!	Resource loader.
 */
class MCD_ABSTRACT_CLASS MCD_CORE_API IResourceLoader
{
public:
	enum LoadingState
	{
		NotLoaded = 0 << 0,			//! The resource is not loaded yet.
		Loading = 1 << 0,			//! Loading operation is in progress.
		PartialLoaded = 1 << 1,		//!	Some data is loaded and can be display immediately (progressive loading).
		Loaded = 1 << 2,			//! The resource is fully loaded.
		Aborted = 1 << 3,			//! The loading operation is aborted (eg error occurred)
		Stopped = Loaded + Aborted,	//! For user to test against stopped ((state & Stopped) == true)
		CanCommit = PartialLoaded + Loaded	//! For user to test against when data is available to commit ((state & CanCommit) == true)
	};	// LoadingState

	virtual ~IResourceLoader() {}

	/*!	The flagship function of the loader.
		\code
		while(true) {
			if(loader.load(is) == IResourceLoader::PartialLoaded) {
				loader.commit(resource);
				// Do something useful on the partial loaded data
				// ...
			} else
			if(loader.load(is) == IResourceLoader::Loaded) {
				loader.commit(resource);
				break;
			}
			else
			if(loader.load(is) == IResourceLoader::Aborted) {
				// Error handling code here
				// ...
				break;
			}
		}
		\endcode
	 */
	virtual sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const wchar_t* args=nullptr) = 0;

	//! Commit the data buffer in the loader to the resource.
	virtual void commit(Resource& resource) = 0;

	virtual LoadingState getLoadingState() const = 0;

	/*!	Callback that will be invoked by a concrete IResourceManager when a resource loader
		finished part of the progressive load in NON-BLOCKING mode.

		The default implementation of this function will schedule the load immediatly:
		manager.reSchedule(context, priority, args);

		\param context Pass this context (now or later) to IResourceManager::reSchedule() and
			the manager will continue the loading of that resource.
		\note If the context haven't pass to IResourceManager::reSchedule(), then the user should
			make sure the context will pass to IResourceManager::cancelSchedule() to prevent memory leak.
	 */
	virtual void onPartialLoaded(IResourceManager& manager, sal_in void* context, uint priority, sal_in_z_opt const wchar_t* args);
};	// IResourceLoader

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCELOADER__
