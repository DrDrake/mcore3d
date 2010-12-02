#ifndef __MCD_CORE_SYSTEM_RESOURCE__
#define __MCD_CORE_SYSTEM_RESOURCE__

#include "Atomic.h"
#include "Path.h"
#include "IntrusivePtr.h"
#include "WeakPtr.h"

namespace MCD {

class ResourceManager;

/// Resource
class MCD_CORE_API Resource : public IntrusiveSharedWeakPtrTarget<AtomicInteger>, Noncopyable
{
public:
	explicit Resource(const Path& fileId);

// Attributes
	const Path& fileId() const {
		return mFileId;
	}

	/// To track how many times the resource is committed (by loader or any other means).
	size_t commitCount() const { return mCommitCount; }

protected:
	/*!	Virtual function to make Resource a polymorphic type so
		that we can apply dynamic_cast on concrete resource type.
	 */
	virtual ~Resource();

protected:
	friend class ResourceManager;
	Path mFileId;
	AtomicInteger mCommitCount;
};	// Resource

typedef IntrusivePtr<Resource> ResourcePtr;
typedef IntrusiveSharedWeakPtr<Resource> ResourceWeakPtr;

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCE__
