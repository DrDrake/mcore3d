#ifndef __MCD_CORE_ENTITY_COMPONENT__
#define __MCD_CORE_ENTITY_COMPONENT__

#include "../ShareLib.h"
#include "../System/WeakPtr.h"
#include <typeinfo>

namespace MCD {

class Entity;

/*!	Base class for everything attached to Entity.
 */
class MCD_ABSTRACT_CLASS MCD_CORE_API Component : public WeakPtrTarget
{
public:
	Component();

	virtual ~Component() {}

	/*!	Each Component should belongs to one family.
		For example, a MeshComponent and SkyboxComponent are both inherit from RenderableComponent
		and they all return typeid(RenderableComponent) as the family type.
	 */
	virtual const std::type_info& familyType() const = 0;

// Attributes
	//! The Entity that this component belongs to.
	sal_maybenull Entity* entity();

protected:
	friend class Entity;

	//! The Entity that this component belongs to.
	Entity* mEntity;
};	// Component

/*!	We use weak pointer to reference a Component.
	It's too easy to make cyclic-reference hell if we use reference counted pointer.
 */
typedef WeakPtr<Component> ComponentPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_COMPONENT__
