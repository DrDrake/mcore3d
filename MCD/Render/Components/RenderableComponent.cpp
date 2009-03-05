#include "Pch.h"
#include "RenderableComponent.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

void RenderableComponent::traverseEntities(Entity* entityNode)
{
	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		RenderableComponent* renderable = itr->findComponent<RenderableComponent>();
		if(renderable)
			renderable->render();

		itr.next();
	}
}

}	// namespace MCD
