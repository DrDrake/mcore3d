#ifndef __MCD_CORE_ENTITY_ENTITY__
#define __MCD_CORE_ENTITY_ENTITY__

#include "Component.h"
#include "../Math/Mat44.h"
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
class MCD_CORE_API Entity : public IntrusiveSharedWeakPtrTarget<size_t>, Noncopyable
{
public:
	explicit Entity(sal_in_z_opt const char* name=nullptr);

	/// Virtual destructor to make this class polymorphic.
	virtual ~Entity();

// Operations
	/// Make this entity a child of \em parent.
	void asChildOf(sal_in Entity* parent);

	/// Append an entity as the FIRST child of this.
	/// Returns the added child
	sal_notnull Entity* addFirstChild(sal_in Entity* child);

	/// Append an entity as the LAST child of this.
	/// Returns the added child
	sal_notnull Entity* addLastChild(sal_in Entity* child);

	/// The input parameter should not be the root node.
	void insertBefore(sal_in Entity* sibling);

	void insertAfter(sal_in Entity* sibling);

	/*!	Find a component in the Entity with the supplied familyType.
		Returns null if none is found.
	 */
	sal_maybenull Component* findComponent(const type_info& familyType) const;

	///	Wrap over findComponent() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponent(const std::type_info& familyType) const {
		return polymorphic_downcast<T*>(findComponent(familyType));
	}
	template<class T>
	sal_maybenull T* findComponent() const {
		return findComponent<T>(typeid(T));
	}

	/*!	Find a component in the Entity with the supplied typeid.
		Returns null if none is found.
	 */
	sal_maybenull Component* findComponentExactType(const std::type_info& type) const;

	///	Wrap over findComponentExactType() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponentExactType(const std::type_info& type) const {
		return polymorphic_downcast<T*>(findComponentExactType(type));
	}
	template<class T>
	sal_maybenull T* findComponentExactType() const {
		return findComponentExactType<T>(typeid(T));
	}

	// TODO: Replace findComponentInChildren with some filtering iterator.
	/*!	Returns the Component of type familyType in the Entity or any of its children.
		Returns null if none is found.
		\todo Decide which tree traversal is the best to use.
	 */
	sal_maybenull Component* findComponentInChildren(const std::type_info& familyType) const;

	///	Wrap over findComponentInChildren() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponentInChildren(const std::type_info& familyType) const {
		return polymorphic_downcast<T*>(findComponentInChildren(familyType));
	}
	template<class T>
	sal_maybenull T* findComponentInChildren() const {
		return findComponentInChildren<T>(typeid(T));
	}

	sal_maybenull Component* findComponentInChildrenExactType(const std::type_info& type) const;

	///	Wrap over findComponentInChildrenExactType() with polymorphic_downcast
	template<class T>
	sal_maybenull T* findComponentInChildrenExactType(const std::type_info& type) const {
		return polymorphic_downcast<T*>(findComponentInChildrenExactType(type));
	}
	template<class T>
	sal_maybenull T* findComponentInChildrenExactType() const {
		return findComponentInChildrenExactType<T>(typeid(T));
	}

	///	Instead of simply returning true or false, the no. of level is returned.
	size_t isAncestorOf(const Entity& e) const;

	///	Return the youngest common ancestor of the 2 entities, null if none.
	static sal_maybenull Entity* commonAncestor(const Entity& e1, const Entity& e2);

	/*!	Return the firstly found Entity along the siblings, with the name supplied.
		Returns null if none is found.
	 */
	sal_maybenull Entity* findEntityInSibling(sal_in_z const char* name) const;

	/*!	Return the firstly found Entity under the descendants (including indirect children), with the name supplied.
		Returns null if none is found.
		\note This entity will also be considered.
	 */
	sal_maybenull Entity* findEntityInDescendants(sal_in_z const char* name) const;

	/*!	Use a file system like path syntax to local an entity in the entity tree.
		This entity will be returned if empty string is supplied.
		\note Linear complexity.
	 */
	sal_maybenull Entity* findEntityByPath(sal_in_z const char* path) const;

	/*!	Comput the relative path from one Entity to this Entity.
		Returns empty string when both Entity are just the same, or they are not in the same tree.
	 */
	std::string getRelativePathFrom(const Entity& e) const;

	/*!	Add a new component into the Entity.
		Only a single instance is allowed for each family type of Component,
		so the old one (if any) will be deleted before the new one is added.
		If the supplied component is already added to some other Entity, the
		add operation will be canceled.
		\return The added component
	 */
	template<class T>
	sal_notnull T* addComponent(sal_in T* component) {
		return static_cast<T*>(_addComponent(component));
	}

	/*!	Remove the component from this Entity.
		The component will be also deleted, so it is wise to use the
		ComponentPtr (which is a weak pointer) as a reference to any
		Component.
	 */
	void removeComponent(const std::type_info& familyType);

	/*!	Create and return a deep copy of this Entity.
		Please notice that the following will NOT be copied:
		- userData
		- non-cloneable Components
		\note This function is implemented using recursion.
	 */
	virtual sal_notnull Entity* clone() const;

	virtual void destroyThis();

	///	A SAL friendy version of destroyThis().
	static void destroy(sal_maybenull Entity*& entity);

	/// For the scripting system to increment it's reference count to the Entity
	void scriptAddReference();

	/// For the scripting system to decrement it's reference count to the Entity
	void scriptReleaseReference();

// Attributes
	bool enabled;

	std::string name;

	sal_maybenull Entity* parent();
	sal_maybenull Entity* parent() const;

	sal_maybenull Entity* firstChild();
	sal_maybenull Entity* firstChild() const;

	sal_maybenull Entity* lastChild();
	sal_maybenull Entity* lastChild() const;

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

	typedef LinkList<Component> Components;
	Components components;

	/// A handy way to get the root of the main Entity tree
	static sal_maybenull Entity* currentRoot();

	static void setCurrentRoot(sal_maybenull Entity* e);

// Script binding
	void* scriptVm;

	/*!	We use a char buffer to represent a HSQOBJECT object,
		to erase the dependency of squirrel headers. Static
		assert is performed on the cpp to assert the buffer
		size is always valid.
	 */
	char scriptHandle[sizeof(void*) * 2];

protected:
	///	Temporary unlink this entity from it's parent (if any).
	/// The unlinked Entity should attach to another Entity afterward
	/// to ensure all Entity are owned by a root node.
	void unlink();

	/// Generate a default name if this Entity doesn't have one
	void generateDefaultName();

	Component* _addComponent(sal_in_opt Component* component);

	///	Helper function for clone().
	virtual sal_notnull Entity* recursiveClone() const;

	/// Pointer to make the entity hierarchy
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
