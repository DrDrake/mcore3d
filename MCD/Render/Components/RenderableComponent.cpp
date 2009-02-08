#include "Pch.h"
#include "RenderableComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Utility.h"

namespace MCD {

void RenderableComponent::traverseEntities(Entity* entityNode)
{
	for(EntityPreorderIterator itr(entityNode); !itr.ended(); itr.next())
	{
		RenderableComponent* renderable = polymorphic_downcast<RenderableComponent*>(itr->findComponent(typeid(RenderableComponent)));
		if(!renderable)
			continue;

		renderable->render();
	}
}

}	// namespace MCD
