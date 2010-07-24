#include "Pch.h"
#include "../Renderer.h"

namespace MCD {

class RendererComponent::Impl {};

RendererComponent::RendererComponent()
	: mImpl(*new Impl)
{
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
