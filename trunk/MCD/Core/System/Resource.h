#ifndef __MCD_CORE_SYSTEM_RESOURCE__
#define __MCD_CORE_SYSTEM_RESOURCE__

#include "Path.h"
#include "IntrusivePtr.h"
#include "WeakPtr.h"

namespace MCD {

/*!	Resource
 */
class MCD_CORE_API Resource : public WeakPtrTarget
{
public:
	explicit Resource(const Path& fileId);

	const Path& fileId() const {
		return mFileId;
	}

	friend void intrusivePtrAddRef(Resource* resource) {
		++(resource->mRefCount);
	}

	friend void intrusivePtrRelease(Resource* resource)
	{
		if(--(resource->mRefCount) == 0)
			delete resource;
	}

protected:
	/*!	Virtual function to make Resource a polymorhpic type so
		that we can apply dynamic_cast on concret resource type.
	 */
	virtual ~Resource() {}

protected:
	size_t mRefCount;
	Path mFileId;
};	// Resource

typedef IntrusivePtr<Resource> ResourcePtr;

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_RESOURCE__
