#ifndef __MCD_COMPONENT_PREFABLOADERCOMPONENT__
#define __MCD_COMPONENT_PREFABLOADERCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"

namespace MCD {

class DynamicsWorld;
class IResourceManager;
typedef IntrusivePtr<class Prefab> PrefabPtr;

/*!	Monitor the loading progress of the Prefab.
	Once the load operation is finished, the Entity tree will clone to the PrefabLoaderComponent
	attached Entity.
 */
class MCD_COMPONENT_API PrefabLoaderComponent : public BehaviourComponent
{
public:
	PrefabLoaderComponent();

// Cloning
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
		const char* filePath,
		sal_in_z_opt const char* args=nullptr,
		sal_in_opt DynamicsWorld* dynamicsWorld=nullptr
	);

// Attributes
	PrefabPtr prefab;

	bool isLoaded() const;

protected:
	//!	If this counter doesn't match that of prefab, the Entity tree will be refreshed.
	size_t mCommitCount;
};	// PrefabLoaderComponent

typedef IntrusiveWeakPtr<PrefabLoaderComponent> PrefabLoaderComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_PREFABLOADERCOMPONENT__
