#include "Pch.h"
#include "Entity.h"
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

IComponent* Entity::findComponent(const std::type_info& type)
{
	MCD_FOREACH(const IComponent& c, mComponents) {
		if(typeid(c) == type)
			return const_cast<IComponent*>(&c);
	}

	return nullptr;
}

void Entity::addComponent(IComponent* component)
{
	if(!component)
		return;

	removeComponent(typeid(*component));
	mComponents.push_back(component);
}

void Entity::removeComponent(const std::type_info& type)
{
	for(Components::iterator i=mComponents.begin(); i!=mComponents.end(); ++i) {
		if(typeid(*i) == type) {
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

}	// namespace MCD
