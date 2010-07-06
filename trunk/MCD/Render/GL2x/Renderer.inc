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
class MeshComponent2;
class QuadComponent;
class QuadRenderer;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

struct RenderItem
{
	sal_notnull MeshComponent2* mesh;
	sal_notnull IMaterialComponent* material;
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
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;
	sal_notnull RendererComponent* mBackRef;
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
