#include "Pch.h"
#include "RenderModifierComponent.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

/*
void RenderModifierComponent::traverseEntities(Entity* entityNode)
{
	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		RenderModifierComponent* renderable = itr->findComponent<RenderModifierComponent>();
		if(renderable)
			renderable->render();

		itr.next();
	}
}
*/

}	// namespace MCD
