#ifndef __MCD_COMPONENT_PREFABLOADERCOMPONENT__
#define __MCD_COMPONENT_PREFABLOADERCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"

namespace MCD {

class DynamicsWorld;
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

	void reload();

	/*!	Client code responsible for ownership of the returning Entity.
		\param	Static rigid body will be create for all the mesh in the prefab, if a DynamicsWorld is supplied.
	 */
	// TODO: Change DynamicsWorld into a component and use weak pointer.
	static sal_maybenull Entity* loadEntity(
		IResourceManager& resourceManager,
		const wchar_t* filePath,
		sal_in_opt DynamicsWorld* dynamicsWorld=nullptr
	);

// Attributes
	EntityPrototypePtr prefab;

	bool isLoaded() const;

protected:
	bool mLoaded;
};	// PrefabLoaderComponent

typedef IntrusiveWeakPtr<PrefabLoaderComponent> PrefabLoaderComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_PREFABLOADERCOMPONENT__
