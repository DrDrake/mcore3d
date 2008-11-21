#ifndef __MCD_CORE_SYSTEM_RESOURCELOADER__
#define __MCD_CORE_SYSTEM_RESOURCELOADER__

#include "Platform.h"
#include <iosfwd>

namespace MCD {

class Resource;
class Path;

/*!	Resource loader.
 */
class MCD_ABSTRACT_CLASS IResourceLoader
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
				breadk;
			}
			else
			if(loader.load(is) == IResourceLoader::Aborted) {
				// Error handling code here
				// ...
				breadk;
			}
		}
		\endcode
	 */
	virtual sal_checkreturn LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr) = 0;

	//! Commit the data buffer in the loader to the resource.
	virtual void commit(Resource& resource) = 0;

	virtual LoadingState getLoadingState() const = 0;
};	// IResourceLoader

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCELOADER__
