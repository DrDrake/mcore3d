#include "Pch.h"
#include "Resource.h"

namespace MCD {

Resource::Resource(const Path& fileId)
	:
	mRefCount(0),
	mFileId(fileId)
{
}

}	// namespace MCD
