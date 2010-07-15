#include "Pch.h"
#include "PrefabLoaderComponent.h"
#include "Physics/RigidBodyComponent.h"
#include "Prefab.h"
#include "../Core/Entity/Entity.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

PrefabLoaderComponent::PrefabLoaderComponent()
	: mCommitCount(0)
{}

bool PrefabLoaderComponent::cloneable() const {
	return true;
}

Component* PrefabLoaderComponent::clone() const
{
	PrefabLoaderComponent* cloned = new PrefabLoaderComponent;
	cloned->prefab = prefab;
	return cloned;
}

void PrefabLoaderComponent::update(float dt)
{
	Entity* e = entity();
	if(!e || !prefab)
		return;

	// Check the loading status
	if(prefab->commitCount == mCommitCount || !prefab->entity.get())
		return;

	// Remove all existing child nodes first
	while(Entity* child = e->firstChild())
		child->destroyThis();

	// Clone the Entity tree from the prefab
	std::auto_ptr<Entity> cloned(prefab->entity->clone());
	for(Entity* i=cloned->firstChild(); i;) {
		Entity* bk = i;
		i = i->nextSibling();
		bk->asChildOf(e);
	}

	mCommitCount = prefab->commitCount;

	// TODO: PrefabLoaderComponent may remove itself in final release.
}

void PrefabLoaderComponent::reload() {
	mCommitCount = 0;
}

bool PrefabLoaderComponent::isLoaded() const {
	return mCommitCount > 0;
}

Entity* PrefabLoaderComponent::loadEntity(IResourceManager& resourceManager, const char* filePath, const char* args, DynamicsWorld* dynamicsWorld)
{
	PrefabPtr prefab = dynamic_cast<Prefab*>(resourceManager.load(filePath, IResourceManager::NonBlock, 0, args).get());
	if(!prefab)
		return nullptr;

	EntityPtr e = new Entity();
	e->name = filePath;

	// Add component
	PrefabLoaderComponentPtr c = new PrefabLoaderComponent;
	c->prefab = prefab;
	e->addComponent(c.get());

	//! A callback that create static physics collision mesh upon loads complete.
	struct CreatePhysics : public ResourceManagerCallback
	{
		sal_override void doCallback()
		{
			if(prefab && prefab->entity.get() && dynamicsWorld)
				createStaticRigidBody(*dynamicsWorld, *prefab->entity);
		}

		PrefabPtr prefab;
		DynamicsWorld* dynamicsWorld;
	};	// CreatePhysics

	if(dynamicsWorld) {
		CreatePhysics* callback = new CreatePhysics();
		callback->prefab = prefab;
		callback->dynamicsWorld = dynamicsWorld;
		callback->addDependency(filePath);
		resourceManager.addCallback(callback);
	}

	return e.get();
}

}	// namespace MCD
