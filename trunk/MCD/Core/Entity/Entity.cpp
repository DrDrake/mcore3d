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
	if(!mParent)	// If it's the root node, supress the assertion in IntrusiveWeakPtrTarget
		destructionLock();

	unlink();

	Entity* children = mFirstChild;

	// TODO: Rethink about the ownership of Entity
	if(children) do {
		Entity* next = children->mNextSibling;
		children->destructionLock();
		delete children;
		children = next;
	} while(children);
}

void Entity::asChildOf(Entity* parent)
{
	MCD_ASSERT(parent);
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

Entity* Entity::commonAncestor(const Entity& e1, const Entity& e2)
{
	int level1 = 0, level2 = 0;
	for(const Entity* e = &e1; e; e=e->parent())
		++level1;
	for(const Entity* e = &e2; e; e=e->parent())
		++level2;

	size_t commonLevel = level1 < level2 ? level1 : level2;

	const Entity *p1 = &e1, *p2 = &e2;

	for(int i=level1-commonLevel; i--; )
		p1 = p1->parent();
	for(int i=level2-commonLevel; i--; )
		p2 = p2->parent();

	// Go up to the parent until both become the same
	for(size_t i=commonLevel; i--; ) {
		if(p1 == p2)
			return const_cast<Entity*>(p1);
		MCD_ASSUME(p1);
		MCD_ASSUME(p2);
		p1 = p1->parent();
		p2 = p2->parent();
	}

	return nullptr;
}

Entity* Entity::findEntityInSibling(sal_in_z const char* name) const
{
	for(Entity* e=const_cast<Entity*>(this); e; e=e->nextSibling()) {
		if(e->name == name)
			return e;
	}

	return nullptr;
}

Entity* Entity::findEntityInDescendants(const char* name) const
{
	for(EntityPreorderIterator itr(const_cast<Entity*>(this)); !itr.ended(); itr.next()) {
		if(itr->name == name)
			return itr.current();
	}

	return nullptr;
}

Entity* Entity::findEntityByPath(const char* path) const
{
	PathIterator itr(path);
	const Entity* e = this;

	do {
		std::string s = itr.next(false);
		if(s.empty())
			break;

		// Remove trailing slash
		if(s[s.size() - 1] == '/')
			s.resize(s.size() - 1);

		if(s == "..")
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

std::string Entity::getRelativePathFrom(const Entity& from) const
{
	std::string ret;
	const Entity* lcp = nullptr;
	size_t levelDiff = 0;

	// Find the most descend (lowest) common parent
	if((lcp = commonAncestor(*this, from)) != nullptr) {
		for(const Entity* e=this; e != lcp && e; e = e->parent())
			++levelDiff;
		for(const Entity* e=&from; e != lcp && e; e = e->parent())
			ret += "../";
	}
	else	// The 2 incomming nodes are of seperated tree.
		return "";

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
			ret += tmp[i]->name + "/";

		MCD_STACKFREE(tmp);
	}

	return ret;
}

void Entity::addComponent(Component* component)
{
	if(!component)
		return;

	if(component->entity() != nullptr) {
		Log::format(Log::Warn, "The component is already added to an Entity");
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
			c->destroyThis();
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

Entity* Entity::recursiveClone() const
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
		Entity* newChild = child->recursiveClone();	// Note that we call recursiveClone() recursively
		newChild->mParent = newEnt.get();
		if(child == mFirstChild)
			newEnt->mFirstChild = newChild;
		else
			lastChild->mNextSibling = newChild;

		lastChild = newChild;
	}

	return newEnt.release();
}

Entity* Entity::clone() const
{
	Entity* ret = recursiveClone();

	// Perform the postClone() operation on all components.
	for(ComponentPreorderIterator itr(ret); !itr.ended(); itr.next())
		MCD_VERIFY(itr->postClone(*this, *ret));

	return ret;
}

void Entity::destroyThis()
{
	if(!this)	// NOTE: Make this function behaive like the delete operator: do nothing no null.
		return;
	if(mParent)
		destructionLock();
	delete this;
}

void Entity::destroy(Entity*& entity)
{
	if(entity)
		entity->destroyThis();
	entity = nullptr;
}

EntityPreorderIterator::EntityPreorderIterator(Entity* start)
	: mCurrent(start), mStart(start)
{}

EntityPreorderIterator::EntityPreorderIterator(Entity* start, Entity* current)
	: mCurrent(current), mStart(start)
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

int EntityPreorderIterator::offsetFrom(const Entity& from, const Entity& to)
{
	int i = 0;
	for(EntityPreorderIterator itr(const_cast<Entity*>(&from)); !itr.ended(); itr.next(), ++i) {
		if(itr.current() != &to) continue;
		return i;
	}
	return -1;
}

Entity* EntityPreorderIterator::entityByOffset(const Entity& entity, int offset)
{
	int i = 0;
	for(EntityPreorderIterator itr(const_cast<Entity*>(&entity)); !itr.ended(); itr.next(), ++i) {
		if(i == offset)
			return itr.current();
	}
	return nullptr;
}

}	// namespace MCD
