#include "Pch.h"
#include "Entity.h"
#include "Component.h"
#include "../System/Utility.h"

namespace MCD {

Entity::Entity()
	: enabled(true),
	  mParent(nullptr), mFirstChild(nullptr), mNextSlibing(nullptr),
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

void Entity::asChildOf(Entity* parent)
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

void Entity::insertBefore(sal_in Entity* slibing)
{
	if(!slibing || !slibing->mParent)
		return;

	// Unlink this Entity first
	unlink();

	Entity* node = slibing->mParent->mFirstChild;
	if(node == slibing) {
		slibing->mParent->mFirstChild = this;
		mParent = slibing->mParent;
		mNextSlibing = slibing;
		return;
	}

	// Find out which node to insert after,
	// since we didn't store a previous slibing pointer
	while(node) {
		if(node->mNextSlibing == slibing)
			break;
		node = node->mNextSlibing;
	}

	// Should never be null since slibing should contained by slibing->mParent->mFirstChild
	MCD_ASSUME(node != nullptr);

	insertAfter(node);
}

void Entity::insertAfter(sal_in Entity* slibing)
{
	if(!slibing)
		return;

	// Unlink this Entity first
	unlink();

	mParent = slibing->mParent;

	Entity* old = slibing->mNextSlibing;
	slibing->mNextSlibing = this;
	mNextSlibing = old;
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

Component* Entity::findComponent(const std::type_info& familyType) const
{
	// NOTE: For simplicity, only linear search is used right now.
	for(const Component* c = components.begin(); c != components.end(); c = c->next()) {
		if(c->familyType() == familyType)
			return const_cast<Component*>(c);
	}

	return nullptr;
}

Component* Entity::findComponentInChildren(const std::type_info& familyType) const
{
	for(EntityPreorderIterator itr(const_cast<Entity*>(this)); !itr.ended(); itr.next()) {
		Component* ret = itr->findComponent(familyType);
		if(ret)
			return ret;
	}

	return nullptr;
}

Entity* Entity::findEntityInChildren(const wchar_t* name) const
{
	for(EntityPreorderIterator itr(const_cast<Entity*>(this)); !itr.ended(); itr.next()) {
		if(itr->name == name)
			return itr.current();
	}

	return nullptr;
}

void Entity::addComponent(Component* component)
{
	if(!component)
		return;

	removeComponent(component->familyType());
	components.pushBack(*component);
	component->mEntity = this;
}

void Entity::removeComponent(const std::type_info& familyType)
{
	for(Component* c = components.begin(); c != components.end(); c = c->next()) {
		if(c->familyType() == familyType) {
			delete c;
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

Entity* Entity::nextSibling() {
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
			return mCurrent = mCurrent->firstChild();
		else if(mCurrent->nextSibling())
			return mCurrent = mCurrent->nextSibling();
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

Entity* EntityPreorderIterator::nextEnabled()
{
	next();

	if(mCurrent && !mCurrent->enabled)
		skipChildren();

	return mCurrent;
}

Entity* EntityPreorderIterator::skipChildren()
{
	// The following borrows the code from next() function, where noChildMove is always true
	while(mCurrent)
	{
		if(mCurrent->nextSibling())
			return mCurrent = mCurrent->nextSibling();

		mCurrent = mCurrent->parent();
		if(mCurrent == mStart)
			mCurrent = nullptr;
	}

	return mCurrent;
}

}	// namespace MCD
