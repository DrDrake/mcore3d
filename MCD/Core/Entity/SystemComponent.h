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
public:
	explicit FileSystemComponent(FileSystemCollection& fs) : fileSystem(fs)	{}

// Attributes
	FileSystemCollection& fileSystem;
};	// FileSystemComponent

class IResourceManager;

class MCD_CORE_API ResourceManagerComponent : public SystemComponent
{
public:
	explicit ResourceManagerComponent(IResourceManager& mgr) : resourceManager(mgr) {}

// Attributes
	IResourceManager& resourceManager;
};	// ResourceManagerComponent

class TaskPool;

class MCD_CORE_API TaskPoolComponent : public SystemComponent
{
public:
	explicit TaskPoolComponent(TaskPool& pool) : taskPool(pool)	{}

// Attributes
	TaskPool& taskPool;
};	// TaskPoolComponent

}	// namespace MCD

#endif	// __MCD_SYSTEM_SYSTEMCOMPONENT__

