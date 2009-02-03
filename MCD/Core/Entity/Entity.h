#ifndef __MCD_CORE_ENTITY_ENTITY__
#define __MCD_CORE_ENTITY_ENTITY__

#include "../ShareLib.h"
#include "../Math/Mat44.h"
#include "../System/PtrVector.h"
#include <typeinfo>

namespace MCD {

class Component;

/*!	Entity is the basic unit of a game object, act as a container of Component.
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

	sal_maybenull Component* findComponent(const std::type_info& type);

	/*!	Add a new component into the Entity.
		Only a single instance is allowed for each type of Component,
		so the old one (if any) will be deleted before the new one is added.
	 */
	void addComponent(sal_in Component* component);

	/*!	Remove the component from this Entity.
		The component will be also deleted, so it is wise to use the
		ComponentPtr (which is a weak pointer) as a reference to any
		Component.
	 */
	void removeComponent(const std::type_info& type);

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

class MCD_CORE_API EntityContainer
{
public:
};	// EntityContainer

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_ENTITY__
