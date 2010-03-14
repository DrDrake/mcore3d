#ifndef __MCD_CORE_ENTITY_ENTITY__
#define __MCD_CORE_ENTITY_ENTITY__

#include "Component.h"
#include "../Math/Mat44.h"
#include "../System/ScriptOwnershipHandle.h"
#include "../System/UserData.h"
#include "../System/Utility.h"
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
class MCD_CORE_API Entity : public IntrusiveWeakPtrTarget, Noncopyable
{
public:
	Entity();

	//! Virtual destructor to make this class polymorphic.
	virtual ~Entity();

// Operations
	//! Make this entity a child of \em parent.
	void asChildOf(sal_in Entity* parent);

	//! The input parameter should not be the root node.
	void insertBefore(sal_in Entity* sibling);

	void insertAfter(sal_in Entity* sibling);

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

	//!	Wrap over findComponent() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponent(const std::type_info& familyType) const {
		return polymorphic_downcast<T*>(findComponent(familyType));
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

	//!	Wrap over findComponentInChildren() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponentInChildren(const std::type_info& familyType) const {
		return polymorphic_downcast<T*>(findComponentInChildren(familyType));
	}
	template<class T>
	sal_maybenull T* findComponentInChildren() const {
		return findComponentInChildren<T>(typeid(T));
	}

	//!	Instead of simply returning true or false, the no. of level is returned.
	size_t isAncestorOf(const Entity& e) const;

	/*!	Return the firstly found Entity along the siblings, with the name supplied.
		Returns null if none is found.
	 */
	sal_maybenull Entity* findEntityInSibling(sal_in_z const wchar_t* name) const;

	/*!	Return the firstly found Entity under the descendants (including indirect children), with the name supplied.
		Returns null if none is found.
		\note This entity will also be considered.
	 */
	sal_maybenull Entity* findEntityInDescendants(sal_in_z const wchar_t* name) const;

	/*!	Use a file system like path syntax to local an entity in the entity tree.
		This entity will be returned if empty string is supplied.
		\note Linear complexity.
	 */
	sal_maybenull Entity* findEntityByPath(sal_in_z const wchar_t* path) const;

	/*!	Comput the relative path from one Entity to this Entity.
		Returns empty string when both Entity are just the same, or they are not in the same tree.
	 */
	std::wstring getRelativePathFrom(const Entity& e) const;

	/*!	Add a new component into the Entity.
		Only a single instance is allowed for each family type of Component,
		so the old one (if any) will be deleted before the new one is added.
		If the supplied component is already added to some other Entity, the
		add operation will be canceled.
	 */
	void addComponent(sal_in_opt Component* component);

	/*!	Remove the component from this Entity.
		The component will be also deleted, so it is wise to use the
		ComponentPtr (which is a weak pointer) as a reference to any
		Component.
	 */
	void removeComponent(const std::type_info& familyType);

	/*!	Create and return a deep copy of this Entity.
		Please notice that the following will NOT be copied:
		- userData
		- scriptOwnershipHandle
		- non-cloneable Components
		\note This function is implemented using recursion.
	 */
	virtual sal_notnull Entity* clone() const;

	virtual void destroyThis();

	//!	A SAL friendy version of destroyThis().
	static void destroy(sal_maybenull Entity*& entity);

// Attributes
	bool enabled;

	std::wstring name;

	sal_maybenull Entity* parent();
	sal_maybenull Entity* parent() const;

	sal_maybenull Entity* firstChild();
	sal_maybenull Entity* firstChild() const;

	sal_maybenull Entity* nextSibling();
	sal_maybenull Entity* nextSibling() const;

	Mat44f localTransform;

	/*!	The world transform is calculated by chaining up all parent's
		localTransform and it's own.
	 */
	Mat44f worldTransform() const;

	/*!	It will modify the localTransform such that the outcomming
		worldTransform is what you want.
	 */
	void setWorldTransform(const Mat44f& transform);

	UserData userData;

	ScriptOwnershipHandle scriptOwnershipHandle;

	typedef LinkList<Component> Components;
	Components components;

protected:
	/*!	When preforming some "move" operation, we don't want the unlink() function
		release the strong script reference.
	 */
	void unlink(bool keepScriptStrongReference);

	//! Pointer to make the entity hierarchy
	Entity* mParent, *mFirstChild, *mNextSibling;
};	// Entity

/*!	We use weak pointer to reference an Entity.
	The ownership of an Entity is determined by it's parent, it means
	the root node of an Entity tree handle the lifetime of all it's children.
	For external reference, we should use a weak reference.
 */
typedef IntrusiveWeakPtr<Entity> EntityPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_ENTITY__
