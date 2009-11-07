#ifndef __MCD_CORE_SYSTEM_RESOURCELOADER__
#define __MCD_CORE_SYSTEM_RESOURCELOADER__

#include "Platform.h"
#include "../ShareLib.h"
#include <iosfwd>

namespace MCD {

class Path;
class Resource;

/*	A context for the user of IResourceLoader to control when to continue a progressive load.
	\note One of the concret implementation of IPartialLoadContext is developed inside ResourceManager.

	By design, the ownership of IPartialLoadContext should give away once continueLoad()
	is invoked. Following is an example usage:
	\code
	void MyLoader::onPartialLoaded(IPartialLoadContext& context, uint priority, const wchar_t* args) {
		// Backup the context for later use.
		mContext.reset(&context);
	}
	void MyLoader::TimeToContinueLoad() {
		if(!mContext.get()) return;
		// IMPORTANT: Release the auto_ptr since it's ownership is passed to the
		// concret implementation behind IPartialLoadContext
		mContext.release()->continueLoad(0, nullptr);
	}
	std::auto_ptr<IPartialLoadContext> mContext;
	\endcode
 */
class MCD_ABSTRACT_CLASS IPartialLoadContext
{
public:
	virtual ~IPartialLoadContext() {}

	virtual void continueLoad(uint priority, sal_in_z_opt const wchar_t* args) = 0;
};	// IPartialLoadContext

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

	/*!	User of IResourceLoader should invoke this function when IResourceLoader::load()
		returns a state with IResourceLoader::PartialLoaded, and pass it with a concret
		implementation of IPartialLoadContext, such that the derived class of IResourceLoader
		has a chance to control when the progressive load should continue.

		The derived class of IResourceLoader has the responsibility to delete the context,
		if the context has never be continue it's loading.

		\note The default implementation of this function will continue the load immediatly:
			context.continueLoad(priority, args);
	 */
	virtual void onPartialLoaded(IPartialLoadContext& context, uint priority, sal_in_z_opt const wchar_t* args);
};	// IResourceLoader

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCELOADER__
