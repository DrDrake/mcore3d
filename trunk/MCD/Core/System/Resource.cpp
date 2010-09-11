#include "Pch.h"
#include "Resource.h"

namespace MCD {

Resource::Resource(const Path& fileId)
	: mFileId(fileId), mCommitCount(0)
{}

}	// namespace MCD
