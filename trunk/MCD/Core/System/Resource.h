#ifndef __MCD_CORE_SYSTEM_RESOURCE__
#define __MCD_CORE_SYSTEM_RESOURCE__

#include "Atomic.h"
#include "Path.h"
#include "IntrusivePtr.h"
#include "WeakPtr.h"

namespace MCD {

/*!	Resource
 */
class MCD_CORE_API Resource : public WeakPtrTarget, public IntrusiveSharedObject<AtomicInteger>, Noncopyable
{
public:
	explicit Resource(const Path& fileId);

	const Path& fileId() const {
		return mFileId;
	}

protected:
	/*!	Virtual function to make Resource a polymorphic type so
		that we can apply dynamic_cast on concrete resource type.
	 */
	virtual ~Resource() {}

protected:
	Path mFileId;
};	// Resource

typedef IntrusivePtr<Resource> ResourcePtr;
typedef WeakPtr<Resource> ResourceWeakPtr;

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCE__
