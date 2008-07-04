#ifndef __SGE_CORE_SYSTEM_RESOURCELOADER__
#define __SGE_CORE_SYSTEM_RESOURCELOADER__

#include "Platform.h"
#include <iosfwd>

namespace SGE {

class Resource;

/*!	Resource loader.
 */
class SGE_NOVTABLE IResourceLoader
{
public:
	enum LoadingState
	{
		NotLoaded = 0 << 0,			//! The resource is not loaded yet.
		Loading = 1 << 0,			//! Loading operation is in progress.
		PartialLoaded = 1 << 1,		//!	Some data is loaded and can be display immediatly (progressive loading).
		Loaded = 1 << 2,			//! The resource is fully loaded.
		Aborted = 1 << 3,			//! The loading operation is aborted (eg error occured)
		Stopped = Loaded + Aborted,	//! For user to test against stopped ((state & Stopped) == true)
		CanCommit = PartialLoaded + Loaded	//! For user to test against when data is available to commit ((state & CanCommit) == true)
	};	// LoadingState

	virtual ~IResourceLoader() {}

	/*!	The flagship function of the loader.
		\code
		while(true) {
			if(loader.load(is) == IResourceLoader::PartialLoaded) {
				loader.commit(resource);
				// Do something usefull on the partial loaded data
				// ...
			} else
			if(loader.load(is) == IResourceLoader::Loaded) {
				loader.commit(resource);
				breadk;
			}
			else
			if(loader.load(is) == IResourceLoader::Aborted) {
				// Eror handling code here
				// ...
				breadk;
			}
		}
		\endcode
	 */
	virtual sal_checkreturn LoadingState load(sal_maybenull std::istream* is) = 0;

	//! Commit the data buffer in the loader to the resource.
	virtual void commit(Resource& resource) = 0;

	virtual LoadingState getLoadingState() const = 0;
};	// IResourceLoader

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_RESOURCELOADER__
