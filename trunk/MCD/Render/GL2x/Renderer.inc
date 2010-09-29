#ifndef __MCD_RENDER_GL2X_RENDERER__
#define __MCD_RENDER_GL2X_RENDERER__

#include "../Renderer.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Math/Vec2.h"
#include "../../Core/Math/Vec3.h"
#include "../../Core/System/Array.h"
#include "../../Core/System/Deque.h"
#include "../../Core/System/Map.h"
#include <vector>

namespace MCD {

class LightComponent;
class IMaterialComponent;
class MeshComponent;
class QuadComponent;
class QuadRenderer;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent> CameraComponentPtr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

struct RenderItem
{
	sal_maybenull Entity* entity;
	sal_notnull IDrawCall* drawCall;
	sal_notnull IMaterialComponent* material;
	Mat44f worldTransform;
};	// RenderItem

struct RenderItemNode : public MapBase<float>::Node<RenderItemNode>
{
	typedef MapBase<float>::Node<RenderItemNode> Super;
	RenderItemNode(float depth, const RenderItem& item)
		: Super(depth), mRenderItem(item)
	{}
	RenderItem mRenderItem;
};	// RenderItemNode

typedef Map<RenderItemNode> RenderItems;

class RendererComponent::Impl
{
public:
	Impl();

	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void render(Entity& entityTree);

	void processRenderItems(RenderItems& items);

	void preRenderMaterial(size_t pass, IMaterialComponent& mtl);
	void postRenderMaterial(size_t pass, IMaterialComponent& mtl);

	//! mDefaultCamera and mCurrentCamera may differ from each other when rendering using light's view
	CameraComponentPtr mDefaultCamera, mCurrentCamera;
	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;
	sal_notnull RendererComponent* mBackRef;

	IMaterialComponent* mLastMaterial;	//!< To reduce material setup cost
	std::stack<IMaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mTransparentQueue, mOpaqueQueue;

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;

	struct QuadMaterialPair { QuadComponent* quad; IMaterialComponent* mtl; };
	typedef std::vector<QuadMaterialPair> Quads;
	Quads mQuads;
	std::auto_ptr<QuadRenderer> mQuadRenderer;

	//!	Variables used when traversing the entity tree
	EntityPreorderIterator mEntityItr;
	IMaterialComponent* mCurrentMaterial;

	std::vector<RenderableComponentPtr> mRenderables;
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
