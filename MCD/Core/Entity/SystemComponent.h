#ifndef __MCD_SYSTEM_SYSTEMCOMPONENT__
#define __MCD_SYSTEM_SYSTEMCOMPONENT__

#include "../ShareLib.h"
#include "Component.h"

namespace MCD {

/// A special kind of component that stores system necessary objects
/// For example: file system, resource manager, thread pool etc...
class MCD_ABSTRACT_CLASS MCD_CORE_API SystemComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(SystemComponent);
	}
};	// Component

class FileSystemCollection;

class MCD_CORE_API FileSystemComponent : public SystemComponent
{
	friend class Framework;
	FileSystemCollection& mFileSystemCollection;
	
public:
	explicit FileSystemComponent(FileSystemCollection& fs)
		: mFileSystemCollection(fs)
	{}

// Attributes
	FileSystemCollection& fileSystem() {
		return mFileSystemCollection;
	}
};	// FileSystemComponent

class IResourceManager;

class MCD_CORE_API ResourceManagerComponent : public SystemComponent
{
	friend class Framework;
	IResourceManager& mResourceManager;
	
public:
	explicit ResourceManagerComponent(IResourceManager& resourceManager)
		: mResourceManager(resourceManager)
	{}

// Attributes
	IResourceManager& resourceManager() {
		return mResourceManager;
	}
};	// ResourceManagerComponent

}	// namespace MCD

#endif	// __MCD_SYSTEM_SYSTEMCOMPONENT__

