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

RendererComponent& RendererComponent::current()
{
	return *reinterpret_cast<RendererComponent*>(nullptr);
}

void RendererComponent::begin()
{
}

void RendererComponent::end(float dt)
{
}

}	// namespace MCD
