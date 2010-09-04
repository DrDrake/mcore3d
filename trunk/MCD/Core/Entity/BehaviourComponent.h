#ifndef __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__
#define __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__

#include "Component.h"

namespace MCD {

class MCD_ABSTRACT_CLASS MCD_CORE_API BehaviourComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(BehaviourComponent);
	}

	//! The derived components should override this function for defining behaviour.
	virtual void update(float dt) = 0;

	//! Invoke the BehaviourComponent::update() in every Entity under the entityNode sub-tree.
	static void traverseEntities(sal_maybenull Entity* entityNode, float dt);
};	// BehaviourComponent

typedef IntrusiveWeakPtr<BehaviourComponent> BehaviourComponentPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__
