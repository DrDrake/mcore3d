#include "Pch.h"
#include "BehaviourComponent.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

void BehaviourComponent::traverseEntities(Entity* entityNode)
{
	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		BehaviourComponent* behaviour = itr->findComponent<BehaviourComponent>();
		if(behaviour)
			behaviour->update();

		itr.next();
	}
}

}	// namespace MCD
