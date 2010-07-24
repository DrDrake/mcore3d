#include "Pch.h"
#include "../QuadComponent.h"
#include "Renderer.inc"

namespace MCD {

void QuadComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	IMaterialComponent* m = renderer.mMaterialStack.top();
	RendererComponent::Impl::QuadMaterialPair pair = { this, m };
	renderer.mQuads.push_back(pair);
}

}	// namespace MCD
