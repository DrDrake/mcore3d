#pragma once

#include "../../MCD/Core/System/Platform.h"
#undef nullptr

namespace MCD {

class IResourceManager;

}	// namespace MCD

namespace Binding {

class ResourceManagerImpl;
ref class FileSystemCollection;

/*!	A place for everyone to access
 */
public ref class ResourceManager
{
public:
	ResourceManager(FileSystemCollection^ fileSystemCollection);

// Operations
	//! Force destroy the implementation, since .Net dispose is not so deterministic.
	void destroy();

	void pollForUpdatedFiles();

// Attributes
	sal_notnull MCD::IResourceManager* getRawPtr();

	property FileSystemCollection^ fileSystemCollection {
		FileSystemCollection^ get();
	}

protected:
	~ResourceManager();
	!ResourceManager();

	ResourceManagerImpl* mImpl;
};	// ResourceManager

}	// namespace Binding
