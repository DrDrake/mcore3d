#include "Pch.h"
#include "BehaviourComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Utility.h"

namespace MCD {

void BehaviourComponent::traverseEntities(Entity* entityNode)
{
	for(EntityPreorderIterator itr(entityNode); !itr.ended(); itr.next())
	{
		if(!itr->enabled)
			continue;

		BehaviourComponent* behaviour = polymorphic_downcast<BehaviourComponent*>(itr->findComponent(typeid(BehaviourComponent)));
		if(!behaviour)
			continue;

		behaviour->update();
	}
}

}	// namespace MCD
