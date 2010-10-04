#ifndef __MCD_RENDER_GL2X_RENDERER__
#define __MCD_RENDER_GL2X_RENDERER__

#include "../Renderer.h"
#include "../Renderer.inc"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Math/Vec2.h"
#include "../../Core/Math/Vec3.h"
#include "../../Core/System/Array.h"
#include <vector>

namespace MCD {

class RendererComponent::Impl : public RendererCommon
{
public:
	Impl();

	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void render(Entity& entityTree);

	void processRenderItems(RenderItems& items, IDrawCall::Statistic& statistic, size_t& materialSwitch);

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;
};	// Impl

}	// namespace MCD

#endif	// __MCD_RENDER_GL2X_RENDERER__
