#include "Pch.h"
#include "BehaviourComponent.h"
#include "Entity.h"
#include "../System/MemoryProfiler.h"

namespace MCD {

void BehaviourComponent::traverseEntities(Entity* entityNode, float dt)
{
	MemoryProfiler::Scope profiler("BehaviourComponent::traverseEntities");

	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		BehaviourComponent* behaviour = itr->findComponent<BehaviourComponent>();

		// NOTE: We must iterate to the next entity first, since
		// update() may delete the current Entity.
		itr.next();

		if(behaviour)
			behaviour->update(dt);
	}
}

}	// namespace MCD
