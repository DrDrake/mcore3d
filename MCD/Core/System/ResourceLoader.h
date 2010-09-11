#ifndef __MCD_CORE_SYSTEM_RESOURCELOADER__
#define __MCD_CORE_SYSTEM_RESOURCELOADER__

#include "Macros.h"
#include "Path.h"
#include "TaskPool.h"
#include "WeakPtr.h"
#include <iosfwd>
#include <vector>

namespace MCD {

class Path;
class ResourceManager;
typedef IntrusivePtr<class Resource> ResourcePtr;
typedef IntrusiveSharedWeakPtr<Resource> ResourceWeakPtr;

typedef IntrusivePtr<class IResourceLoader> IResourceLoaderPtr;
typedef IntrusiveSharedWeakPtr<IResourceLoader> IResourceLoaderWeakPtr;

/// Resource loader.
class MCD_ABSTRACT_CLASS MCD_CORE_API IResourceLoader
	: public IntrusiveSharedWeakPtrTarget<AtomicInteger>
	, private MCD::TaskPool::Task
{
public:
	enum LoadingState
	{
		NotLoaded = 0 << 0,			///< The resource is not loaded yet.
		Loading = 1 << 0,			///< Loading operation is in progress.
		PartialLoaded = 1 << 1,		///< Some data is loaded and can be display immediately (progressive loading).
		Loaded = 1 << 2,			///< The resource is fully loaded.
		Aborted = 1 << 3,			///< The loading operation is aborted (eg error occurred)
		Stopped = Loaded + Aborted,	///< For user to test against stopped ((state & Stopped) == true)
		CanCommit = PartialLoaded + Loaded	///< For user to test against when data is available to commit ((state & CanCommit) == true)
	};	// LoadingState

	IResourceLoader();

// Operations
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
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr, sal_in_z_opt const char* args=nullptr) = 0;

	/// Commit the data buffer in the loader to the resource.
	virtual void commit(Resource& resource) = 0;

	void continueLoad();

	/// Call this to inform resource of this loader is depends on another resource.
	void dependsOn(const IResourceLoaderPtr& loader);

// Attrubites
	const Path& fileId() const;

	/// Get the resource that this loader loading to.
	ResourcePtr resource() const;

	size_t loadCount() const;

	LoadingState loadingState() const;

	/// Default is non-blocking, concret class can override this.
	virtual int defaultBlockingIteration() const { return 0; }

	/// Force the blocing iteration no matter what option the user pass to ResourceManager::load().
	virtual int forceBlockingIteration() const { return -1; }

	/// \note All dependency functions only accounts for direct dependency.

	/// Number of resource that depending on this.
	size_t dependencyParentCount() const;

	sal_maybenull IResourceLoaderPtr getDependencyParent(size_t index) const;

	size_t dependencyChildCount() const;

	sal_maybenull IResourceLoaderPtr getDependencyChild(size_t index) const;

protected:
	virtual ~IResourceLoader() {}

	mutable Mutex mMutex;

private:
	friend class ResourceManager;

	void releaseThis();

	sal_override void run(Thread& thread);

	LoadingState _load(const Path* fileId, sal_in_z const char* args);

	/// Use Path as the key, note that we should store a copy of the Path in PathKey rather than a reference,
	/// because the resource can be destroyed at any time but the PathKey is destroyed at a later time.
	struct PathKey : public MapBase<Path, const Path&>::Node<PathKey>
	{
		typedef MapBase<Path, const Path&>::Node<PathKey> Super;
		explicit PathKey(const Path& path) : Super(path) {}
		MCD_DECLAR_GET_OUTER_OBJ(IResourceLoader, mPathKey);
		sal_override void destroyThis();
	};	// PathKey

	/// Explicity store the fileId path as the Map's key, since mResource
	/// may become null at any time.
	PathKey mPathKey;

	size_t mLoadCount;
	LoadingState mState;

	std::string mArgs;
	ResourceWeakPtr mResource;				///< Note that we use weak pointer here
	ResourceManager* mResourceManager;
	std::auto_ptr<std::istream> mIStream;

	// Intermediate states
	bool mNeedEnqueu;						///< Prevent summitting a loader into the task pool multiple times
	bool mPendForCommit;
	size_t mOutstandingContinueCount;		///< Make sure no (multiple) call to continueLoad() will ignored

	std::vector<IResourceLoaderWeakPtr> mDepenseOn, mDepenseBy;
};	// IResourceLoader

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCELOADER__
