#ifndef __MCD_CORE_ENTITY_COMPONENT__
#define __MCD_CORE_ENTITY_COMPONENT__

#include "../ShareLib.h"
#include "../System/LinkList.h"
#include "../System/ScriptOwnershipHandle.h"
#include "../System/WeakPtr.h"
#include <typeinfo>

namespace MCD {

class Entity;

/*!	Base class for everything attached to Entity.
 */
class MCD_ABSTRACT_CLASS MCD_CORE_API Component :
	public WeakPtrTarget, public LinkListBase::Node<Component>
{
public:
	Component();

	virtual ~Component() {}

	/*!	Each Component should belongs to one family.
		For example, a MeshComponent and SkyboxComponent are both inherit from RenderableComponent
		and they all return typeid(RenderableComponent) as the family type.
	 */
	virtual const std::type_info& familyType() const = 0;

	//!	Returns true if this Component is cloneable during Entity::clone().
	virtual sal_checkreturn bool cloneable() const { return false; }

	/*!	Creates and returns a deep copy of this Component.
		This method should returns nullptr if this Component is not cloneable.
	 */
	virtual sal_maybenull Component* clone() const { return nullptr; }

// Attributes
	//! The Entity that this component belongs to.
	sal_maybenull Entity* entity() const;

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
typedef WeakPtr<Component> ComponentPtr;

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
		return mStart == nullptr && mCurrent == nullptr;
	}

	//! Returns the next element in the collection, and advances to the next.
	sal_maybenull Component* next();

protected:
	Component* mCurrent;
	Entity* mStart;
};	// ComponentPreorderIterator

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_COMPONENT__
