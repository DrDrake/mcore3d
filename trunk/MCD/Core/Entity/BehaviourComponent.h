#ifndef __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__
#define __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__

#include "Component.h"
#include <vector>

namespace MCD {

class MCD_ABSTRACT_CLASS MCD_CORE_API BehaviourComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(BehaviourComponent);
	}

	//! The derived components should override this function for defining behaviour.
	virtual void update(float dt) = 0;

protected:
	sal_override void gather();
};	// BehaviourComponent

typedef IntrusiveWeakPtr<BehaviourComponent> BehaviourComponentPtr;

class MCD_CORE_API BehaviourUpdaterComponent : public ComponentUpdater
{
	friend class BehaviourComponent;

protected:
	sal_override void begin();
	sal_override void end(float dt);

	std::vector<BehaviourComponentPtr> mComponents;
};	// BehaviourUpdaterComponent

typedef IntrusiveWeakPtr<BehaviourUpdaterComponent> BehaviourUpdaterComponentPtr;

}	// namespace MCD

#endif	// __MCD_CORE_ENTITY_BEHAVIOURCOMPONENT__
