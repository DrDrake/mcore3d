#ifndef __MCD_CORE_ENTITY_PREFAB__
#define __MCD_CORE_ENTITY_PREFAB__

#include "../ShareLib.h"
#include "../../Core/System/Resource.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

class ResourceManager;

/*!	A kind of resource that owns a tree of Entity, it can be regarded as a pre-fabricated
	game object which then cloned for use.
 */
class MCD_CORE_API Prefab : public Resource
{
public:
	explicit Prefab(const Path& fileId);

	//! When an Prefab is loaded, \em entity is not be NULL
	std::auto_ptr<Entity> entity;

protected:
	sal_override ~Prefab();
};	// Prefab

typedef IntrusivePtr<Prefab> PrefabPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_PREFAB__
