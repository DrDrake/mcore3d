#include "Pch.h"
#include "SystemComponent.h"
#include "BehaviourComponent.h"
#include "Entity.h"
#include "../System/Resource.h"
#include "../System/ResourceLoader.h"
#include "../System/ResourceManager.h"

namespace MCD {

void ResourceManagerComponent::update(Timer* timer, float timeOut)
{
	const bool performLoad = mResourceManager.taskPool().getThreadCount() == 0 || performLoadInMainThread;

	while(IResourceLoaderPtr loader = mResourceManager.popEvent(timer, timeOut, performLoad))
	{
		size_t parentCount = loader->dependencyParentCount();

		for(size_t pi=0; pi<=parentCount; ++pi)
		{
			const IResourceLoaderPtr d = parentCount ? loader->getDependencyParent(pi) : loader;
			if(!d)
				continue;

			for(Callbacks::iterator i=mCallbacks.begin(); i!=mCallbacks.end();)
			{
				if(fulfillRequirement(*d, *i)) {
					i->component->entity()->enabled = true;
					mCallbacks.erase(i++);
				}
				else
					++i;
			}
		}
	}
}

bool ResourceManagerComponent::fulfillRequirement(const IResourceLoader& loader, const Callback& callback) const
{
	if(loader.fileId() != callback.path)
		return false;

	const ResourcePtr r = loader.resource();
	if(!r)
		return false;

	// Check for its dependency
	if(callback.isRecursive)
	{
		const size_t dependCount = loader.dependencyChildCount();
		for(size_t i=0; i<dependCount; ++i)
		{
			if(const IResourceLoaderPtr d = loader.getDependencyChild(i)) {
				Callback c2 = { d->fileId(), false, 1, nullptr };

				// Reusing fulfillRequirement()
				if(!fulfillRequirement(*d, c2))
					return false;
			}
		}
	}

	// Check for itself
	if(loader.loadingState() & IResourceLoader::Stopped)
		return true;

	const size_t blockIteration = callback.minLoadIteration < 0 ? loader.defaultBlockingIteration() : callback.minLoadIteration;
	if(loader.loadCount() >= blockIteration && r->commitCount() > 0)
		return true;

	return false;
}

void ResourceManagerComponent::registerCallback(const Path& fileId, BehaviourComponent& behaviour, bool isRecursive, int minLoadIteration)
{
	if(Entity* e = behaviour.entity()) {
		Callback callback = { fileId, isRecursive, minLoadIteration, &behaviour };

		// Check if the resource is already loaded
		if(IResourceLoaderPtr loader = mResourceManager.getLoader(fileId)) {
			if(fulfillRequirement(*loader, callback)) {
				e->enabled = true;
				return;
			}
		}

		e->enabled = false;
		mCallbacks.push_back(callback);
	}
}

}	// namespace MCD
