#include "Pch.h"
#include "Entity.h"
#include "Component.h"
#include "../System/Log.h"
#include "../System/Utility.h"

namespace MCD {

Entity::Entity()
	: enabled(true),
	  mParent(nullptr), mFirstChild(nullptr), mNextSibling(nullptr),
	  localTransform(Mat44f::cIdentity)
{
}

Entity::~Entity()
{
	unlink();

	Entity* children = mFirstChild;

	// TODO: Rethink about the ownership of Entity
	if(children) do {
		Entity* next = children->mNextSibling;
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
	mNextSibling = oldFirstChild;
	parent->mFirstChild = this;

	scriptOwnershipHandle.useStrongReference(true);
}

void Entity::insertBefore(sal_in Entity* sibling)
{
	if(!sibling || !sibling->mParent)
		return;

	// Unlink this Entity first
	unlink();

	Entity* node = sibling->mParent->mFirstChild;
	if(node == sibling) {
		sibling->mParent->mFirstChild = this;
		mParent = sibling->mParent;
		mNextSibling = sibling;
		return;
	}

	// Find out which node to insert after,
	// since we didn't store a previous sibling pointer
	while(node) {
		if(node->mNextSibling == sibling)
			break;
		node = node->mNextSibling;
	}

	// Should never be null since sibling should contained by sibling->mParent->mFirstChild
	MCD_ASSUME(node != nullptr);

	insertAfter(node);
}

void Entity::insertAfter(sal_in Entity* sibling)
{
	if(!sibling)
		return;

	// Unlink this Entity first
	unlink();

	mParent = sibling->mParent;

	Entity* old = sibling->mNextSibling;
	sibling->mNextSibling = this;
	mNextSibling = old;

	scriptOwnershipHandle.useStrongReference(true);
}

void Entity::unlink()
{
	if(!mParent)
		return;

	if(mParent->mFirstChild == this) {
		mParent->mFirstChild = mNextSibling;
	} else {
		// Find the previous sibling
		Entity* previous = mParent->mFirstChild;
		Entity* next;
		while((next = previous->mNextSibling) != nullptr) {
			if(next == this)
				break;
			previous = next;
		}

		previous->mNextSibling = mNextSibling;
	}

	mParent = nullptr;
	mNextSibling = nullptr;
	// The children are keep intact
	// mFirstChild = mFirstChild;

	scriptOwnershipHandle.useStrongReference(false);
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

	if(component->entity() != nullptr) {
		Log::format(Log::Warn, L"The component is already added to an Entity");
		return;
	}

	removeComponent(component->familyType());
	components.pushBack(*component);
	component->mEntity = this;

	component->onAdd();
	component->scriptOwnershipHandle.useStrongReference(true);
}

void Entity::removeComponent(const std::type_info& familyType)
{
	for(Component* c = components.begin(); c != components.end(); c = c->next()) {
		if(c->familyType() == familyType) {
			c->onRemove();
			delete c;
			return;
		}
	}
}

Entity* Entity::parent() {
	return mParent;
}

Entity* Entity::firstChild() {
	return mFirstChild;
}

Entity* Entity::nextSibling() {
	return mNextSibling;
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

void Entity::setWorldTransform(const Mat44f& transform)
{
	if(mParent)
		localTransform = transform * mParent->worldTransform().inverse();
	else
		localTransform = transform;
}

sal_notnull Entity* Entity::clone() const
{
	std::auto_ptr<Entity> newEnt(new Entity());
	
	newEnt->enabled = enabled;
	newEnt->name = name;
	newEnt->localTransform = localTransform;

	// Copy all cloneable components
	for(const Component* comp = components.begin(); comp != components.end(); comp = comp->next())
	{
		Component* newComp = comp->clone();

		if(nullptr != newComp)
			newEnt->addComponent(newComp);
	}

	Entity* lastChild = nullptr;
	// Clone the children Entity
	for(Entity* child = mFirstChild; nullptr != child; child = child->nextSibling())
	{
		Entity* newChild = child->clone();	// Note that we call clone() recursively
		newChild->mParent = newEnt.get();
		if(child == mFirstChild)
			newEnt->mFirstChild = newChild;
		else
			lastChild->mNextSibling = newChild;

		lastChild = newChild;
	}

	return newEnt.release();
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
