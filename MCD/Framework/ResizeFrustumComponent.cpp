#include "Pch.h"
#include "ResizeFrustumComponent.h"
#include "../Render/Camera.h"
#include "../Render/Frustum.h"
#include "../Render/RenderTarget.h"

namespace MCD {

ResizeFrustumComponent::ResizeFrustumComponent()
	: mOldWidth(0), mOldHeight(0)
{}

void ResizeFrustumComponent::update(float dt)
{
	if(!camera || !renderTarget)
		return;

	const size_t w = renderTarget->targetWidth();
	const size_t h = renderTarget->targetHeight();

	if(mOldWidth == w && mOldHeight == h)
		return;

	mOldWidth = w;
	mOldHeight = h;
	Frustum& f = camera->frustum;
	if(f.projectionType == Frustum::Perspective) {
		const float aspect = float(w) / h;
		f.create(f.fov(), aspect, f.near, f.far);
	}
	else	// Ortho or YDown2D
		f.create(-0, float(w), 0, float(h), -1, 1);
}

}	// namespace MCD
