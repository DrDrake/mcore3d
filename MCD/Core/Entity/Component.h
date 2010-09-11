#ifndef __MCD_CORE_ENTITY_COMPONENT__
#define __MCD_CORE_ENTITY_COMPONENT__

#include "EntityIterator.h"
#include "../System/LinkList.h"
#include "../System/ScriptOwnershipHandle.h"
#include "../System/WeakPtr.h"
#include <typeinfo>

namespace MCD {

class Entity;

/*!	Base class for everything attached to Entity.

	Deleting a Component will automatically remove itself from the Entity where it attached to.
	You may need to invoke destructionLock() before deleting a Component for thread safty issues
	with weak pointer, or call destroyThis() which handle both destructionLock() and delete.
 */
class MCD_ABSTRACT_CLASS MCD_CORE_API Component :
	public IntrusiveWeakPtrTarget, public LinkListBase::Node<Component>, Noncopyable
{
public:
	Component();

	virtual ~Component();

	/*!	Each Component should belongs to one family.
		For example, a MeshComponent and SkyboxComponent are both inherit from RenderableComponent
		and they all return typeid(RenderableComponent) as the family type.
	 */
	virtual const type_info& familyType() const = 0;

	/*!	Creates and returns a deep copy of this Component.
		This method should returns nullptr if this Component is not cloneable.
	 */
	virtual sal_maybenull Component* clone() const { return nullptr; }

	/*!	After an Entity tree is cloned, a second pass is performed to resolve the
		inter-dependency between all the components, and then reproduce that dependency
		on the new cloned tree.
	 */
	virtual sal_checkreturn bool postClone(const Entity& src, Entity& dest) { return true; }

	//!	Callback function that will invoked just after the component is added to an Entity.
	virtual void onAdd();

	/*!	Callback function that will invoked just before the component is removed from an Entity.
		\note The derived class's callback will not be invoked by default when the Component
			is being destroyed, since virtual function cannot be called inside the base destructor.
			If you need that behaviour, call onRemove() explicitly in the derived class destructor.
	 */
	virtual void onRemove();

	//!	Overrided LinkListBase::NodeBase::destroyThis() for handling IntrusiveWeakPtrTarget::destructionLock().
	sal_override void destroyThis();

// Attributes
	//! The Entity that this component belongs to.
	sal_maybenull Entity* entity() const;

	//! Returns the enabled flag of the containing Entity, false if the Component is not contained by any Entity.
	sal_checkreturn bool enabled() const;

	//! Set the enable flag of the host Entity, do nothing if this Component doesn't belongs to any Entity.
	void setEnabled(bool b);

	ScriptOwnershipHandle scriptOwnershipHandle;

protected:
	friend class Entity;

	/*!	The Entity that this component belongs to.
		There is no need to use EntityPtr, since the Entity itself owns this component.
	 */
	Entity* mEntity;
};	// Component

/*!	We use weak pointer to reference a Component.
	It's too easy to make cyclic-reference hell if we use reference counted pointer.
 */
typedef IntrusiveWeakPtr<Component> ComponentPtr;

/*!	An iterator that traverse over all Component within a certain Entity tree.
	Example:
	\code
	Entity root;
	for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next()) {
		// Do something ...
	}
	\endcode
 */
class MCD_CORE_API ComponentPreorderIterator
{
public:
	explicit ComponentPreorderIterator(sal_maybenull Entity* start);

	ComponentPreorderIterator(sal_maybenull Entity* start, sal_maybenull Component* current);

	// NOTE: Assumming the iterator is valid and so the returned pointer will not be null.
	sal_notnull Component* operator->() {
		return mCurrent;
	}

	//! Return the current element.
	sal_notnull Component* current() {
		return mCurrent;
	}

	//! Returns true if there are NO more items in the collection.
	bool ended() const {
		return mCurrent == nullptr;
	}

	//! Returns the next element in the collection, and advances to the next.
	sal_maybenull Component* next();

	//!	Returns how many preorder iterations should preformed inorder to get component from entity.
	static sal_checkreturn int offsetFrom(const Entity& entity, Component& component);

	//!	Returns a component identified by an offset from an Entity.
	static sal_maybenull Component* componentByOffset(const Entity& entity, int offset);

protected:
	Component* mCurrent;
	EntityPreorderIterator mCurrentEntity;
};	// ComponentPreorderIterator

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_COMPONENT__
