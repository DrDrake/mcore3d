#include "Pch.h"
#include "../RenderTargetComponent.h"
#include "../Renderer.h"
#include "../Camera.h"
#include "../RenderWindow.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

RenderTargetComponent::RenderTargetComponent()
	: clearColor(0, 1)
	, viewPortLeftTop(0), viewPortWidthHeight(0)
	, window(nullptr), mImpl(0)
{
}

RenderTargetComponent::~RenderTargetComponent()
{
}

TexturePtr RenderTargetComponent::createTexture(const GpuDataFormat& format, size_t width, size_t height)
{
	return nullptr;
}

void RenderTargetComponent::render()
{
}

void RenderTargetComponent::render(RendererComponent& renderer, bool swapBuffers)
{
}

}	// namespace MCD
