#include "Pch.h"
#include "RenderableComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Utility.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

void RenderableComponent::traverseEntities(Entity* entityNode)
{
	glMatrixMode(GL_MODELVIEW);

	for(EntityPreorderIterator itr(entityNode); !itr.ended(); itr.next())
	{
		RenderableComponent* renderable = polymorphic_downcast<RenderableComponent*>(itr->findComponent(typeid(RenderableComponent)));
		if(!renderable)
			continue;

		glPushMatrix();
		glMultTransposeMatrixf(itr->worldTransform().getPtr());

		renderable->render();

		glPopMatrix();
	}
}

}	// namespace MCD
