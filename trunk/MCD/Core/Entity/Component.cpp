#include "Pch.h"
#include "Component.h"
#include "Entity.h"
#include "../System/Utility.h"

namespace MCD {

Component::Component()
	: mEntity(nullptr)
{
}

Entity* Component::entity() const {
	return mEntity;
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

}	// namespace MCD
