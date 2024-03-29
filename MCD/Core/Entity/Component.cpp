#include "Pch.h"
#include "Component.h"
#include "Entity.h"
#include "../../../3Party/squirrel/squirrel.h"

// NOTE: For dll export purpose
#include "SystemComponent.h"

namespace MCD {

Component::Component()
	: scriptVm(nullptr), mEntity(nullptr)
{
}

Component::~Component()
{
	MCD_ASSERT(mRefCount == 0);
}

// NOTE: This simply empty is put in cpp otherwise Intel Parallel Studio will
// report false positive memory error.
void Component::onAdd() {}

void Component::onRemove() {}

void Component::destroyThis()
{
	removeThis();
	intrusivePtrRelease(this);
}

Entity* Component::entity() const {
	return mEntity;
}

bool Component::enabled() const {
	return mEntity ? mEntity->enabled : false;
}

void Component::setEnabled(bool b)
{
	if(mEntity)
		mEntity->enabled = b;
}

void Component::scriptAddReference()
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(scriptVm);
	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(scriptHandle);
	if(v) sq_addref(v, h);
}

void Component::scriptReleaseReference()
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(scriptVm);
	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(scriptHandle);
	if(v) sq_release(v, h);
}

void ComponentUpdater::traverseBegin(Entity& entityTree)
{
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		if(ComponentUpdater* updater = itr->findComponent<ComponentUpdater>())
			updater->begin();

		itr.next();
	}
}

void ComponentUpdater::traverseEnd(Entity& entityTree, float dt)
{
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		if(ComponentUpdater* updater = itr->findComponent<ComponentUpdater>())
			updater->end(dt);

		itr.next();
	}
}

ComponentPreorderIterator::ComponentPreorderIterator(Entity* start)
	: mCurrent(nullptr), mCurrentEntity(start)
{
	if(start) {
		mCurrent = start->components.begin();

		// The supplied Entity may contains no Component
		if(start->components.begin() == start->components.end())
			next();
	}
}

ComponentPreorderIterator::ComponentPreorderIterator(Entity* start, Component* current)
	: mCurrent(current), mCurrentEntity(start, current ? current->entity() : nullptr)
{
}

Component* ComponentPreorderIterator::next()
{
	if(!mCurrent)
		return nullptr;

	mCurrent = mCurrent->next();

	// NOTE: mCurrent->next() may be null if mCurrent does not contain any component
	// mCurrent->isInList() is null means the end of component list in mCurrentEntity
	if(mCurrent && !mCurrent->isInList())
		mCurrent = nullptr;

	// Move to next non-empty entity, if needed
	if(!mCurrentEntity.ended()) while(!mCurrent) {
		mCurrentEntity.next();

		if(mCurrentEntity.ended())	// End of all Entities
			break;

		if(mCurrentEntity->components.elementCount() == 0)	// This entity didn't have a component
			continue;

		mCurrent = mCurrentEntity->components.begin();	// Found an Entity with component
		break;
	}

	return mCurrent;
}

int ComponentPreorderIterator::offsetFrom(const Entity& entity, Component& component)
{
	int i = 0;
	for(ComponentPreorderIterator itr(const_cast<Entity*>(&entity)); !itr.ended(); itr.next(), ++i) {
		if(itr.current() != &component) continue;
		return i;
	}
	return -1;
}

Component* ComponentPreorderIterator::componentByOffset(const Entity& entity, int offset)
{
	int i = 0;
	for(ComponentPreorderIterator itr(const_cast<Entity*>(&entity)); !itr.ended(); itr.next(), ++i) {
		if(i == offset)
			return itr.current();
	}
	return nullptr;
}

}	// namespace MCD
