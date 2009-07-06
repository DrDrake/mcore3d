#ifndef __MCD_CORE_ENTITY_ENTITY__
#define __MCD_CORE_ENTITY_ENTITY__

#include "Component.h"
#include "../Math/Mat44.h"
#include "../System/ScriptOwnershipHandle.h"
#include "../System/UserData.h"
#include <string>

namespace MCD {

class Component;

/*!	Entity is the basic unit of a game object, act as a container of Component.

	Entity can link together to form a tree structure using the "parent", "firstChild"
	and "nextSibling" attributes as follows:

	root
	|
	n1---n2---n3
	     |    |
	     n21  n31--n32--n33
 */
class MCD_CORE_API Entity : public WeakPtrTarget
{
public:
	Entity();

	//! Virtual destructor to make this class polymorphic.
	virtual ~Entity();

// Operations
	//! Make this entity a child of \em parent.
	void asChildOf(sal_in Entity* parent);

	//! The input parameter should not be the root node.
	void insertBefore(sal_in Entity* slibing);

	void insertAfter(sal_in Entity* slibing);

	/*!	Unlink this entity from it's parent (if any).
		\note
			This function only detech the sub entity tree from the parent
			and will NOT preform deletion. Remember to handle the ownership
			to prevent memory leaks. To preform unlink and delete, simply
			deleting the Entity and the unlink() will be called implicitly.
	 */
	void unlink();

	/*!	Find a component in the Entity with the supplied familyType.
		Returns null if none is found.
	 */
	sal_maybenull Component* findComponent(const std::type_info& familyType) const;

	//!	Wrap over findComponent() with dynamic_cast
	template<class T>
	sal_maybenull T* findComponent(const std::type_info& familyType) const {
		return dynamic_cast<T*>(findComponent(familyType));
	}
	template<class T>
	sal_maybenull T* findComponent() const {
		return findComponent<T>(typeid(T));
	}

	// TODO: Replace findComponentInChildren with some filtering iterator.
	/*!	Returns the Component of type familyType in the Entity or any of its children.
		Returns null if none is found.
		\todo Decide which tree traversal is the best to use.
	 */
	sal_maybenull Component* findComponentInChildren(const std::type_info& familyType) const;

	//!	Wrap over findComponentInChildren() with dynamic_cast
	template<class T>
	sal_maybenull T* findComponentInChildren(const std::type_info& familyType) const {
		return dynamic_cast<T*>(findComponentInChildren(familyType));
	}
	template<class T>
	sal_maybenull T* findComponentInChildren() const {
		return findComponentInChildren<T>(typeid(T));
	}

	/*!	Return the firstly found Entity undert the children, with the name supplied.
		Returns null if none is found.
		\note This entity will also be considered.
	 */
	sal_maybenull Entity* findEntityInChildren(sal_in_z const wchar_t* name) const;

	/*!	Add a new component into the Entity.
		Only a single instance is allowed for each family type of Component,
		so the old one (if any) will be deleted before the new one is added.
		If the supplied component is already added to some other Entity, the
		add operation will be canceled.
	 */
	void addComponent(sal_in Component* component);

	/*!	Remove the component from this Entity.
		The component will be also deleted, so it is wise to use the
		ComponentPtr (which is a weak pointer) as a reference to any
		Component.
	 */
	void removeComponent(const std::type_info& familyType);

	/*! Create and return a deep copy of this Entity.
		Please notice that the following will NOT be copied:
		- userData
		- scriptOwnershipHandle
		- non-cloneable Components
	*/
	sal_notnull Entity* clone() const;

	Mat44f worldTransform() const;

// Attributes
	bool enabled;

	std::wstring name;

	sal_maybenull Entity* parent();

	sal_maybenull Entity* firstChild();

	sal_maybenull Entity* nextSibling();

	Mat44f localTransform;

	UserData userData;

	ScriptOwnershipHandle scriptOwnershipHandle;

	typedef LinkList<Component> Components;
	Components components;

protected:
	//! Pointer to make the entity hierarchy
	Entity* mParent, *mFirstChild, *mNextSlibing;
};	// Entity

/*!	We use weak pointer to reference an Entity.
	The ownership of an Entity is determined by it's parent, it means
	the root node of an Entity tree handle the lifetime of all it's children.
	For external reference, we should use a weak reference.
 */
typedef WeakPtr<Entity> EntityPtr;

/*!	An iterator that preforms a pre-order traversal on the Entity tree.
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

	// NOTE: Assumming the iterator is valid and so the returned pointer will not be null.
	sal_notnull Entity* operator->() {
		return mCurrent;
	}

	//! Return the current element.
	sal_notnull Entity* current() {
		return mCurrent;
	}

	//! Returns true if there are NO more items in the collection.
	bool ended() const {
		return mCurrent == nullptr;
	}

	//! Returns the next element in the collection, and advances to the next.
	sal_maybenull Entity* next();

	/*!	Returns and advance to the next enabled Entity.
		If an Entity is disabled, all it's children are also skipped.

		\code
		if(node && node->enabled) {	// Take care if the first node is disabled
			for(EntityPreorderIterator itr(node); !itr.ended(); itr.nextEnabled()) {
				// Do something with itr
				// ...
			}
		}
		\endcode
	 */
	sal_maybenull Entity* nextEnabled();

	/*!	Returns and advances to the next element in the collection, without visiting current node's children.
		Usefull when encountering disabled Entity, for example:

		\code
		for(EntityPreorderIterator itr(node); !itr.ended(); ) {
			if(!itr->enabled) {
				itr.skipChildren();
				continue;
			}
			// Do something with itr
			// ...
			itr.next();
		}
		\endcode
	 */
	sal_maybenull Entity* skipChildren();

protected:
	Entity* mCurrent;
	//! The position where this iterator is constructed, so it knows where to stop.
	const Entity* mStart;
};	// EntityPreorderIterator

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_ENTITY__
