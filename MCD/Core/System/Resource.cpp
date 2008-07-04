#include "Pch.h"
#include "Resource.h"

namespace SGE {

Resource::Resource(const Path& fileId)
	:
	mRefCount(0),
	mFileId(fileId)
{
}

}	// namespace SGE
