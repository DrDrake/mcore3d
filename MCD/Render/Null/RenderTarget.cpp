#include "Pch.h"
#include "../RenderTarget.h"
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
{}

RenderTargetComponent::~RenderTargetComponent() {}

TexturePtr RenderTargetComponent::createTexture(const GpuDataFormat& format, size_t width, size_t height)
{
	return nullptr;
}

void RenderTargetComponent::gather() {}

void RenderTargetComponent::render(RendererComponent& renderer) {}

size_t RenderTargetComponent::targetWidth() const { return 0; }

size_t RenderTargetComponent::targetHeight() const { return 0; }

}	// namespace MCD
