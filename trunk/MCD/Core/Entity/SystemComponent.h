#ifndef __MCD_CORE_ENTITY_SYSTEMCOMPONENT__
#define __MCD_CORE_ENTITY_SYSTEMCOMPONENT__

#include "Component.h"
#include "../System/Path.h"
#include <list>

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

class Path;
class IResourceLoader;
class ResourceManager;
class Timer;
typedef IntrusiveWeakPtr<class BehaviourComponent> BehaviourComponentPtr;

class MCD_CORE_API ResourceManagerComponent : public SystemComponent
{
	friend class Framework;
	ResourceManager& mResourceManager;
	
public:
	explicit ResourceManagerComponent(ResourceManager& resourceManager)
		: mResourceManager(resourceManager), performLoadInMainThread(false)
	{}

// Operations
	void registerCallback(const Path& fileId, BehaviourComponent& behaviour, bool isRecursive, int minLoadIteration=-1);

	void update(sal_maybenull Timer* timer=nullptr, float timeOut=0);

// Attributes
	ResourceManager& resourceManager() {
		return mResourceManager;
	}

	/// Instruct the main thread to process loading task.
	/// This variable is ignored if the ResourceManager's thread pool has no thread at all.
	bool performLoadInMainThread;

	/// Search for ResourceManagerComponent from Entity::currentRoot().
	static sal_maybenull ResourceManagerComponent* fromCurrentEntityRoot();

protected:
	struct Callback {
		Path path;
		bool isRecursive;
		int minLoadIteration;
		BehaviourComponentPtr component;
	};	// Callback

	/// Check if a loader does fulfill all the requirements to invoke a callback.
	bool fulfillRequirement(const IResourceLoader& loader, const Callback& callback) const;

	typedef std::list<Callback> Callbacks;
	Callbacks mCallbacks;
};	// ResourceManagerComponent

typedef IntrusiveWeakPtr<class ResourceManagerComponent> ResourceManagerComponentPtr;

class TaskPool;

class MCD_CORE_API TaskPoolComponent : public SystemComponent
{
public:
	explicit TaskPoolComponent(TaskPool& pool) : taskPool(pool)	{}

// Attributes
	TaskPool& taskPool;
};	// TaskPoolComponent

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_SYSTEMCOMPONENT__

