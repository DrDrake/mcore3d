#pragma once

#include "../../MCD/Core/System/ResourceManager.h"
#undef nullptr

namespace Binding {

ref class FileSystemCollection;

/*!	A place for everyone to access
 */
public ref class ResourceManager
{
public:
	ResourceManager(FileSystemCollection^ fileSystemCollection);

	sal_notnull MCD::IResourceManager* getRawPtr();

	//! Force destroy the implementation, since .Net dispose is not so deterministic.
	void destroy();

	property FileSystemCollection^ fileSystemCollection {
		FileSystemCollection^ get();
	}

protected:
	~ResourceManager();
	!ResourceManager();

	MCD::IResourceManager* mImpl;
	FileSystemCollection^ mFileSystemCollection;
};	// ResourceManager

}	// namespace Binding
