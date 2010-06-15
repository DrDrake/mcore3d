#include "Pch.h"
#include "../RenderTargetComponent.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../RenderWindow.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

RenderTargetComponent::RenderTargetComponent()
{}

RenderTargetComponent::~RenderTargetComponent()
{}

WindowRenderTargetComponent::WindowRenderTargetComponent()
	: window(nullptr)
{}

void WindowRenderTargetComponent::render()
{
	if(rendererComponent)
		rendererComponent->mImpl.mRenderTargets.push_back(this);
}

void WindowRenderTargetComponent::render(RendererComponent& renderer)
{
	if(entityToRender && window) {
		window->makeActive();
		window->preUpdate();
		renderer.render(*entityToRender, *this);
		window->postUpdate();
	}
}

TextureRenderTargetComponent::TextureRenderTargetComponent()
{}

TextureRenderTargetComponent::~TextureRenderTargetComponent()
{}

void TextureRenderTargetComponent::render()
{
	if(rendererComponent)
		rendererComponent->mImpl.mRenderTargets.push_back(this);
}

void TextureRenderTargetComponent::render(RendererComponent& renderer)
{
}

}	// namespace MCD
