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

	void processRenderItems(RenderItems& items);

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;

	struct QuadMaterialPair { QuadComponent* quad; IMaterialComponent* mtl; };
	typedef std::vector<QuadMaterialPair> Quads;
	Quads mQuads;
	std::auto_ptr<QuadRenderer> mQuadRenderer;
};	// Impl

class QuadRenderer
{
public:
	QuadRenderer(RendererComponent::Impl& renderer)
		: mRenderer(renderer), mQuadCount(0), mCurrentMaterial(nullptr)
	{}

	/*!	Push a quad to the buffer, the actual render will be performed in flush()
		A change in material or buffer full will triffer a flush automatically.
	 */
	void push(const Mat44f& transform, float width, float height, const Vec4f& uv, IMaterialComponent* mtl);

	void flush();

	RendererComponent::Impl& mRenderer;

	size_t mQuadCount;
	IMaterialComponent* mCurrentMaterial;
	struct Vertex { Vec3f position; Vec2f uv; };
	std::vector<Vertex> mVertexBuffer;
};	// QuadRenderer

}	// namespace MCD

#endif	// __MCD_RENDER_GL2X_RENDERER__
