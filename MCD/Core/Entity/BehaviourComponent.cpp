#include "Pch.h"
#include "BehaviourComponent.h"
#include "Entity.h"
#include "../System/MemoryProfiler.h"

namespace MCD {

void BehaviourComponent::traverseEntities(Entity* entityNode, float dt)
{
	MemoryProfiler::Scope profiler("BehaviourComponent::traverseEntities");

	// Because the update function may remove the component itself, therefore we
	// perform the iteration in a 2 pass manner.
	static std::vector<BehaviourComponentPtr> list;

	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		if(BehaviourComponent* behaviour = itr->findComponent<BehaviourComponent>())
			list.push_back(behaviour);

		itr.next();
	}

	MCD_FOREACH(const BehaviourComponentPtr c, list) {
		if(c)
			c->update(dt);
	}
	list.clear();
}

}	// namespace MCD
