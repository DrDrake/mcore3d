#include "Pch.h"
#include "Renderable.h"
#include "Renderer.h"

namespace MCD {

void RenderableComponent::gather()
{
	RendererComponent::addRenderableToCurrent(*this);
}

}	// namespace MCD
