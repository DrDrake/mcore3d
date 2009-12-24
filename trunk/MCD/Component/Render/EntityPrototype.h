#ifndef __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__
#define __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__

#include "../ShareLib.h"
#include "../../Core/System/Resource.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

class IResourceManager;

class MCD_COMPONENT_API EntityPrototype : public Resource
{
public:
	explicit EntityPrototype(const Path& fileId);

	sal_override ~EntityPrototype();

	//! When an EntityPrototype is loaded, \em entity is not be NULL
	std::auto_ptr<Entity> entity;
};	// EntityPrototype

typedef IntrusivePtr<EntityPrototype> EntityPrototypePtr;

//!	To owns the life-time of EntityPrototype.
class MCD_COMPONENT_API EntityPrototypeComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(EntityPrototypeComponent);
	}

	sal_override sal_maybenull Component* clone() const;

	EntityPrototypePtr entityPrototype;
};	// EntityPrototypeComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_ENTITYPROTOTYPE__
