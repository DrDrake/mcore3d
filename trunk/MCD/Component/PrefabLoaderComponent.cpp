#include "Pch.h"
#include "PrefabLoaderComponent.h"
#include "Render/EntityPrototype.h"
#include "../Core/Entity/Entity.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

PrefabLoaderComponent::PrefabLoaderComponent() : mLoaded(false) {}

bool PrefabLoaderComponent::cloneable() const {
	return true;
}

Component* PrefabLoaderComponent::clone() const
{
	PrefabLoaderComponent* cloned = new PrefabLoaderComponent();
	cloned->prefab = prefab;
	return cloned;
}

void PrefabLoaderComponent::update(float dt)
{
	Entity* e = entity();
	if(mLoaded || !e || !prefab)
		return;

	// Check the loading status
	if(!prefab->entity.get())
		return;

	// Remove all existing child nodes first
	while(Entity* child = e->firstChild())
		child->destroyThis();

	// Clone the Entity tree from the prefab
	for(Entity* i=prefab->entity->firstChild(); i; i = i->nextSibling()) {
		Entity* tmp = i->clone();
		MCD_ASSERT(tmp);
		tmp->asChildOf(e);
	}

	mLoaded = true;

	// TODO: Should PrefabLoaderComponent remove itself?
}

void PrefabLoaderComponent::reload() {
	mLoaded = false;
}

bool PrefabLoaderComponent::isLoaded() const {
	return mLoaded;
}

Entity* PrefabLoaderComponent::loadEntity(IResourceManager& resourceManager, const wchar_t* filePath, bool createCollisionMesh)
{
	EntityPrototypePtr prefab = dynamic_cast<EntityPrototype*>(resourceManager.load(filePath).get());
	if(!prefab)
		return nullptr;

	EntityPtr e = new Entity();
	e->name = filePath;

	// Add component
	PrefabLoaderComponentPtr c = new PrefabLoaderComponent;
	c->prefab = prefab;
	e->addComponent(c.get());

	return e.get();
}

}	// namespace MCD
