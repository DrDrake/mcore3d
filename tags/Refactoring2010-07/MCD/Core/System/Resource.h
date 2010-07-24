#ifndef __MCD_CORE_SYSTEM_RESOURCE__
#define __MCD_CORE_SYSTEM_RESOURCE__

#include "Atomic.h"
#include "Path.h"
#include "WeakPtr.h"

namespace MCD {

/*!	Resource
 */
class MCD_CORE_API Resource : public IntrusiveSharedWeakPtrTarget<AtomicInteger>, Noncopyable
{
public:
	explicit Resource(const Path& fileId);

// Attributes
	const Path& fileId() const {
		return mFileId;
	}

	/*!	To track how many times the resource is committed (by loader or any other means).
		\note Resource loaders should increment this count on the commit stage.
	 */
	size_t commitCount;

protected:
	/*!	Virtual function to make Resource a polymorphic type so
		that we can apply dynamic_cast on concrete resource type.
	 */
	virtual ~Resource() {}

protected:
	Path mFileId;
};	// Resource

typedef IntrusivePtr<Resource> ResourcePtr;
typedef IntrusiveWeakPtr<Resource> ResourceWeakPtr;

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCE__
