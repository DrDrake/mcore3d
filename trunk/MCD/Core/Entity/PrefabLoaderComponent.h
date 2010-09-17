#ifndef __MCD_CORE_ENTITY_PREFABLOADERCOMPONENT__
#define __MCD_CORE_ENTITY_PREFABLOADERCOMPONENT__

#include "Prefab.h"
#include "../../Core/Entity/BehaviourComponent.h"

namespace MCD {

class ResourceManager;
typedef IntrusivePtr<class Prefab> PrefabPtr;

/*!	Monitor the loading progress of the Prefab.
	Once the load operation is finished, the Entity tree will clone to the PrefabLoaderComponent
	attached Entity.
 */
class MCD_CORE_API PrefabLoaderComponent : public BehaviourComponent
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
	 */
	static sal_maybenull Entity* loadEntity(
		ResourceManager& resourceManager,
		const char* filePath,
		sal_in_z_opt const char* args=nullptr
	);

// Attributes
	PrefabPtr prefab;

	bool isLoaded() const;

	size_t commitCount() const { return mCommitCount; }

protected:
	//!	If this counter doesn't match that of prefab, the Entity tree will be refreshed.
	size_t mCommitCount;
};	// PrefabLoaderComponent

typedef IntrusiveWeakPtr<PrefabLoaderComponent> PrefabLoaderComponentPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_PREFABLOADERCOMPONENT__
