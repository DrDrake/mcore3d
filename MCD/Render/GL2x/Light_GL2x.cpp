#include "Pch.h"
#include "../Light.h"
#include "Renderer.inc"

namespace MCD {

void LightComponent::render() {}

void LightComponent::render2(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mLights.push_back(this);
}

}	// namespace MCD
