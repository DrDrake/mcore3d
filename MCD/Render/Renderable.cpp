#include "Pch.h"
#include "Renderable.h"
#include "../Core/Entity/Entity.h"
#include "../Core/System/MemoryProfiler.h"

namespace MCD {

void RenderableComponent::traverseEntities(sal_maybenull Entity* entityNode)
{
	MemoryProfiler::Scope profiler("RenderableComponent::traverseEntities");

	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		RenderableComponent* renderable = itr->findComponent<RenderableComponent>();
		itr.next();

		if(renderable)
			renderable->render();
	}
}

}	// namespace MCD
