#include "Pch.h"
#include "BehaviourComponent.h"
#include "Entity.h"
#include "../System/MemoryProfiler.h"

namespace MCD {

void BehaviourComponent::traverseEntities(Entity* entityNode)
{
	MemoryProfiler::Scope profiler("BehaviourComponent::traverseEntities");

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
