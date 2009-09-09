#include "Pch.h"
#include "RenderableComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/MemoryProfiler.h"

#ifdef MCD_VC
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "glew")
#endif

namespace MCD {

void RenderableComponent::traverseEntities(Entity* entityNode)
{
	MemoryProfiler::Scope profiler("RenderableComponent::traverseEntities");

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
