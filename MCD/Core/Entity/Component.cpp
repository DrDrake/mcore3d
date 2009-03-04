#include "Pch.h"
#include "Component.h"
#include "Entity.h"
#include "../System/Utility.h"

namespace MCD {

Component::Component()
	: mEntity(nullptr)
{
}

Entity* Component::entity() {
	return mEntity;
}

ComponentPreorderIterator::ComponentPreorderIterator(Entity* start)
	: mCurrent(nullptr), mStart(nullptr)
{
	if(start) {
		mStart = start;
		mCurrent = start->components.begin();
	}
}

Component* ComponentPreorderIterator::next()
{
	if(!mCurrent)
		return nullptr;

	mCurrent = mCurrent->next();
	// If the component is in a list, it's next is never null, since there should have a tail node.
	MCD_ASSUME(mCurrent != nullptr);
	if(!mCurrent->isInList())
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
