#include "Pch.h"
#include "../Renderer.h"
//#include "../Light.h"

namespace MCD {

class RendererComponent::Impl {};

RendererComponent::RendererComponent()
	: mImpl(*new Impl)
{
//	LightComponentPtr light = new LightComponent();
//	(void)light;
}

RendererComponent::~RendererComponent()
{
	delete &mImpl;
}

void RendererComponent::render(Entity& entityTree)
{
}

void RendererComponent::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
}

}	// namespace MCD
