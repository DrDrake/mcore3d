#include "Pch.h"
#include "Renderable.h"
#include "../Core/Entity/Entity.h"
#include "../Core/System/MemoryProfiler.h"

namespace MCD {

void RenderableComponent2::traverseEntities(sal_maybenull Entity* entityNode)
{
	MemoryProfiler::Scope profiler("RenderableComponent::traverseEntities");

	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		RenderableComponent2* renderable = itr->findComponent<RenderableComponent2>();
		itr.next();

		if(renderable)
			renderable->render();
	}
}

}	// namespace MCD
