#include "Pch.h"
#include "Resource.h"

namespace MCD {

Resource::Resource(const Path& fileId)
	: mFileId(fileId), commitCount(0)
{}

}	// namespace MCD
