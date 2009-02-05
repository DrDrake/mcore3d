#include "Pch.h"
#include "Entity.h"
#include "Component.h"
#include "../System/Utility.h"

namespace MCD {

Entity::Entity()
	: mParent(nullptr), mFirstChild(nullptr), mNextSlibing(nullptr),
	  localTransform(Mat44f::cIdentity)
{
}

Entity::~Entity()
{
	unlink();

	Entity* children = mFirstChild;

	// TODO: Rethink about the ownership of Entity
	if(children) do {
		Entity* next = children->mNextSlibing;
		delete children;
		children = next;
	} while(children);
}

void Entity::link(Entity* parent)
{
	if(!parent)
		return;

	// Unlink this Entity first
	unlink();

	Entity* oldFirstChild = parent->mFirstChild;
	mParent = parent;
	mNextSlibing = oldFirstChild;
	parent->mFirstChild = this;
}

void Entity::unlink()
{
	if(!mParent)
		return;

	if(mParent->mFirstChild == this) {
		mParent->mFirstChild = mNextSlibing;
	} else {
		// Find the previous slibing
		Entity* previous = mParent->mFirstChild;
		Entity* next;
		while((next = previous->mNextSlibing) != nullptr) {
			if(next == this)
				break;
			previous = next;
		}

		previous->mNextSlibing = mNextSlibing;
	}

	mParent = nullptr;
	mNextSlibing = nullptr;
	// The children are keep intact
	// mFirstChild = mFirstChild;
}

Component* Entity::findComponent(const std::type_info& familyType)
{
	// NOTE: For simplicity, only linear search is used right now.
	MCD_FOREACH(const Component& c, mComponents) {
		if(c.familyType() == familyType)
			return const_cast<Component*>(&c);
	}

	return nullptr;
}

void Entity::addComponent(Component* component)
{
	if(!component)
		return;

	removeComponent(typeid(*component));
	mComponents.push_back(component);
	component->mEntity = this;
}

void Entity::removeComponent(const std::type_info& familyType)
{
	for(Components::iterator i=mComponents.begin(); i!=mComponents.end(); ++i) {
		if(i->familyType() == familyType) {
			i->mEntity = nullptr;
			mComponents.erase(i);
			return;
		}
	}
}

Mat44f Entity::worldTransform() const
{
	Mat44f ret(localTransform);
	Entity* parent = mParent;

	// Traverse all ancestor
	if(parent) do {
		ret = parent->localTransform * ret;
	} while((parent = parent->mParent) != nullptr);

	return ret;
}

Entity* Entity::parent() {
	return mParent;
}

Entity* Entity::firstChild() {
	return mFirstChild;
}

Entity* Entity::nextSlibing() {
	return mNextSlibing;
}


EntityPreorderIterator::EntityPreorderIterator(Entity* start)
	: mCurrent(start), mStart(start)
{}

Entity* EntityPreorderIterator::next()
{
	// After an upward movement is preformed, we will not visit the child again
	bool noChildMove = false;

	while(mCurrent)
	{
		if(mCurrent->firstChild() && !noChildMove)
		{
			return mCurrent = mCurrent->firstChild();
		}
		else if(mCurrent->nextSlibing())
		{
			return mCurrent = mCurrent->nextSlibing();
		}
		else
		{
			mCurrent = mCurrent->parent();
			noChildMove = true;

			if(mCurrent == mStart)
				mCurrent = nullptr;
		}
	}

	return mCurrent;
}

}	// namespace MCD
