#include "Pch.h"
#include "../RenderTargetComponent.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../RenderWindow.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

RenderTargetComponent::RenderTargetComponent()
	: window(nullptr)
{}

RenderTargetComponent::~RenderTargetComponent()
{}

void RenderTargetComponent::render()
{
	if(rendererComponent)
		rendererComponent->mImpl.mRenderTargets.push_back(this);
}

void RenderTargetComponent::render(RendererComponent& renderer)
{
	if(entityToRender && window) {
		window->makeActive();
		window->preUpdate();
		renderer.render(*entityToRender, *this);
		window->postUpdate();
	}
}

}	// namespace MCD
