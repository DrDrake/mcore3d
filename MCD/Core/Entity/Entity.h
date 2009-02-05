#ifndef __MCD_CORE_ENTITY_ENTITY__
#define __MCD_CORE_ENTITY_ENTITY__

#include "../ShareLib.h"
#include "../Math/Mat44.h"
#include "../System/PtrVector.h"
#include <typeinfo>

namespace MCD {

class Component;

/*!	Entity is the basic unit of a game object, act as a container of Component.

	Entity can link together to form a tree structure using the "parent", "firstChild"
	and "nextSlibing" attributes as follows:

	root
	|
	n1---n2---n3
	     |    |
	     n21  n31--n32--n33
 */
class MCD_CORE_API Entity
{
public:
	Entity();
	~Entity();

// Operations
	//! Make this entity a child of \em parent.
	void link(sal_in Entity* parent);

	/*!	Unlink this entity from it's parent (if any).
		\note
			This function only detech the sub entity tree from the parent
			and will NOT preform deletion. Remember to handle the ownership
			to prevent memory leaks. To preform unlink and delete, simply
			deleting the Entity and the unlink() will be called implicitly.
	 */
	void unlink();

	/*!	Find a component in the Entity with the supplied familyType.
		Returns null if non is found.
	 */
	sal_maybenull Component* findComponent(const std::type_info& familyType);

	/*!	Add a new component into the Entity.
		Only a single instance is allowed for each family type of Component,
		so the old one (if any) will be deleted before the new one is added.
	 */
	void addComponent(sal_in Component* component);

	/*!	Remove the component from this Entity.
		The component will be also deleted, so it is wise to use the
		ComponentPtr (which is a weak pointer) as a reference to any
		Component.
	 */
	void removeComponent(const std::type_info& familyType);

	Mat44f worldTransform() const;

// Attributes
	std::wstring name;

	sal_maybenull Entity* parent();

	sal_maybenull Entity* firstChild();

	sal_maybenull Entity* nextSlibing();

	Mat44f localTransform;

protected:
	//! Pointer to make the entity hierarchy
	Entity* mParent, *mFirstChild, *mNextSlibing;

	typedef ptr_vector<Component> Components;
	Components mComponents;
};	// Entity

/*!	An iterator that preforms a pre-order traversal on the Entity tree.
	You can specify
	Example:
	\code
	Entity root;
	for(EntityPreorderIterator itr(&root); !itr.ended(); itr.next()) {
		// Do something ...
	}
	\endcode
 */
class MCD_CORE_API EntityPreorderIterator
{
public:
	explicit EntityPreorderIterator(sal_maybenull Entity* start);

	sal_maybenull Entity* operator->() {
		return mCurrent;
	}

	//! Returns true if there are NO more items in the collection.
	bool ended() const {
		return mCurrent == nullptr;
	}

	//! Returns the next element in the collection, and advances to the next.
	sal_maybenull Entity* next();

protected:
	Entity* mCurrent;
	//! The position where this iterator is constructed, so it knows where to stop.
	const Entity* mStart;
};	// EntityPreorderIterator

class MCD_CORE_API EntityContainer
{
public:
};	// EntityContainer

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_ENTITY__
