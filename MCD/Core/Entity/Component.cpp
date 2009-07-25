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
	: mCurrent(nullptr), mStart(nullptr)
{
	if(start) {
		mStart = start;
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

	// NOTE: mCurrent->next() may be null if mCurrent does not contain any component
	mCurrent = mCurrent->next();

	if(mCurrent && !mCurrent->isInList())
		mCurrent = nullptr;

	// Move to next non-empty entity, if needed
	if(mStart) while(!mCurrent) {
		EntityPreorderIterator i(mStart);
		mStart = i.next();
		if(!mStart) {	// End of all Entities
			mCurrent = nullptr;
			break;
		}
		if(i->components.elementCount() == 0)	// This entity didn't have a component
			continue;
		mCurrent = i->components.begin();	// Found an Entity with component
		break;
	}

	return mCurrent;
}

}	// namespace MCD
