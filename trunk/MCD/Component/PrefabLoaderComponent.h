#ifndef __MCD_COMPONENT_PREFABLOADERCOMPONENT__
#define __MCD_COMPONENT_PREFABLOADERCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"

namespace MCD {

class IResourceManager;
typedef IntrusivePtr<class EntityPrototype> EntityPrototypePtr;

/*!	Monitor the loading progress of the EntityPrototype.
	Once the load operation is finished, the Entity tree will clone to the PrefabLoaderComponent
	attached Entity.
 */
class MCD_COMPONENT_API PrefabLoaderComponent : public BehaviourComponent
{
public:
	PrefabLoaderComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const;

	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void update(float dt);

	//!	Client code responsible for ownership of the returning Entity.
	static sal_maybenull Entity* loadEntity(IResourceManager& resourceManager, const wchar_t* filePath, bool createCollisionMesh);

// Attributes
	EntityPrototypePtr prefab;

	bool isLoaded() const;

protected:
	bool mLoaded;
};	// PrefabLoaderComponent

typedef IntrusiveWeakPtr<PrefabLoaderComponent> PrefabLoaderComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_PREFABLOADERCOMPONENT__
