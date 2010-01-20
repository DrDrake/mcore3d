#include "Pch.h"
#include "Entity.h"
#include "Component.h"
#include "../System/Log.h"
#include "../System/Path.h"
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

	// Unlink this Entity first (keep strong script reference)
	unlink(true);

	Entity* oldFirstChild = parent->mFirstChild;
	mParent = parent;
	mNextSibling = oldFirstChild;
	parent->mFirstChild = this;

	scriptOwnershipHandle.useStrongReference(true);
	scriptOwnershipHandle.removeReleaseHook();
}

void Entity::insertBefore(sal_in Entity* sibling)
{
	if(!sibling)
		return;

	MCD_ASSERT(sibling->mParent && "There should only one a single root node");

	if(!sibling->mParent)
		return;

	// Unlink this Entity first (keep strong script reference)
	unlink(true);

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

	MCD_ASSERT(sibling->mParent && "There should only one a single root node");

	// Unlink this Entity first (keep strong script reference)
	unlink(true);

	mParent = sibling->mParent;

	Entity* old = sibling->mNextSibling;
	sibling->mNextSibling = this;
	mNextSibling = old;

	scriptOwnershipHandle.useStrongReference(true);
	scriptOwnershipHandle.removeReleaseHook();
}

void Entity::unlink()
{
	unlink(false);
}

void Entity::unlink(bool keepScriptStrongReference)
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

	scriptOwnershipHandle.useStrongReference(keepScriptStrongReference);
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

size_t Entity::isAncestorOf(const Entity& e_) const
{
	size_t count = 0;
	for(const Entity* e = e_.parent(); e; e=e->parent(), ++count)
		if(e == this)
			return count + 1;
	return false;
}

Entity* Entity::findEntityInSibling(sal_in_z const wchar_t* name) const
{
	for(Entity* e=const_cast<Entity*>(this); e; e=e->nextSibling()) {
		if(e->name == name)
			return e;
	}

	return nullptr;
}

Entity* Entity::findEntityInDescendants(const wchar_t* name) const
{
	for(EntityPreorderIterator itr(const_cast<Entity*>(this)); !itr.ended(); itr.next()) {
		if(itr->name == name)
			return itr.current();
	}

	return nullptr;
}

Entity* Entity::findEntityByPath(const wchar_t* path) const
{
	PathIterator itr(path);
	const Entity* e = this;

	do {
		std::wstring s = itr.next(false);
		if(s.empty())
			break;

		// Remove trailing slash
		if(s[s.size() - 1] == L'/')
			s.resize(s.size() - 1);

		if(s == L"..")
			e = e->parent();
		else if(e)
		{
			for(e=e->firstChild(); e; e=e->nextSibling())
				if(e->name == s)
					break;
		}
	} while(e);

	return const_cast<Entity*>(e);
}

std::wstring Entity::getRelativePathFrom(const Entity& from) const
{
	std::wstring ret;
	const Entity* lcp = nullptr;
	size_t levelDiff = 0;

	{	// Find the most descend (lowest) common parent
		int thisLevel = 0, fromLevel = 0;
		for(const Entity* e = this; e; e=e->parent())
			++thisLevel;
		for(const Entity* e = &from; e; e=e->parent())
			++fromLevel;

		size_t commonLevel = thisLevel < fromLevel ? thisLevel : fromLevel;

		const Entity *e1 = this, *e2 = &from;

		for(int i=thisLevel-commonLevel; i--; )
			e1 = e1->parent();
		for(int i=fromLevel-commonLevel; i--; ) {
			MCD_ASSUME(e2);
			e2 = e2->parent();
			ret += L"../";
		}

		// Go up to the parent until both become the same
		for(size_t i=commonLevel; i--; ) {
			if(e1 == e2)
				break;
			MCD_ASSUME(e1);
			MCD_ASSUME(e2);
			e1 = e1->parent();
			e2 = e2->parent();
			ret += L"../";
			--commonLevel;	// Note that we haven't interfered the loop
		}

		lcp = e1;
		levelDiff = thisLevel - commonLevel;

		// The 2 incomming nodes are of seperated tree.
		if(!lcp)
			return L"";
	}

	if(levelDiff > 0)
	{	// We need a temporary buffer to reverse the Entities.
		const Entity** tmp = (const Entity**)MCD_STACKALLOCA(sizeof(Entity*) * levelDiff);
		size_t count = 0;

		for(const Entity* e = this; e != lcp; ++count) {
			tmp[count] = e;
			MCD_ASSUME(e);
			e = e->parent();
		}

		MCD_ASSERT(count == levelDiff);

		for(size_t i=count; i--; )
			ret += tmp[i]->name + L"/";

		MCD_STACKFREE(tmp);
	}

	return ret;
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
	component->scriptOwnershipHandle.removeReleaseHook();
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

Entity* Entity::parent() const {
	return mParent;
}

Entity* Entity::firstChild() {
	return mFirstChild;
}

Entity* Entity::firstChild() const {
	return mFirstChild;
}

Entity* Entity::nextSibling() {
	return mNextSibling;
}

Entity* Entity::nextSibling() const {
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
	{	// Try using script handle's clone function first
		ScriptOwnershipHandle dummy;
		// TODO: Add some type safty?
		Entity* e = reinterpret_cast<Entity*>(scriptOwnershipHandle.cloneTo(dummy));
		if(e) {
			e->scriptOwnershipHandle.useStrongReference(true);
			e->scriptOwnershipHandle.removeReleaseHook();
			dummy.setNull();
			return e;
		}
	}

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
