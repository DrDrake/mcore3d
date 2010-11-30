#include "../Camera.h"
#include "../Material.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Deque.h"
#include "../../Core/System/Map.h"

namespace MCD {

class LightComponent;
class IMaterialComponent;
class MeshComponent;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent> CameraComponentPtr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

struct RenderItem
{
	sal_maybenull Entity* entity;
	sal_notnull IDrawCall* drawCall;
	sal_maybenull IMaterialComponent* material;
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

/*!	A common ground for implementing renderer.
	This base class is not strictly necessary, it only aims to simplify the renderer's code.
 */
class RendererCommon
{
public:
	RendererCommon() : mCurrentMaterial(nullptr), mLastMaterial(nullptr) {}

// Operations
	void traverseEntities(Entity& entityTree);

	void submitDrawCall(IDrawCall& drawCall, Entity& entity, const Mat44f& worldTransform);

	void preRenderMaterial(size_t pass, IMaterialComponent& mtl);
	void postRenderMaterial(size_t pass, IMaterialComponent& mtl);

	void resetStatistic();

// Member variables
	sal_notnull RendererComponent* mBackRef;

	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;

	IMaterialComponent *mCurrentMaterial, *mLastMaterial;	//!< To reduce material setup cost
	std::stack<IMaterialComponent*> mMaterialStack;

	CameraComponent* mCurrentCamera;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;

	RenderTargetComponent* mCurrentRenderTarget;

	RenderItems mTransparentQueue, mOpaqueQueue;

	RendererComponent::Statistic mStatistic;
};	// RendererCommon

inline void RendererCommon::traverseEntities(Entity& entityTree)
{
	MCD_ASSERT(!mCurrentMaterial);

	for(EntityPreorderIterator i(&entityTree); !i.ended();)
	{
		// The input parameter entityTree will never skip
		if(!i->enabled && i.current() != &entityTree) {
			i.skipChildren();
			goto CONTINUE;
		}

		Entity* e = i.current();

		// Preform actions defined by the concret type of RenderableComponent we have found
		if(RenderableComponent* renderable = e->findComponent<RenderableComponent>())
			renderable->render(this);

		i.next();

	CONTINUE:
		mMaterialStack.push(mCurrentMaterial);
		// Pop material when moving up (towards parent) or leveling in the tree
		for(int depth = i.depthChange(); depth <= 0; ++depth)
			mMaterialStack.pop();
		mCurrentMaterial = !mMaterialStack.empty() ? mMaterialStack.top() : nullptr;
	}	// Traverse entities

	MCD_ASSERT(!mCurrentMaterial);
	MCD_ASSERT(mMaterialStack.empty());
}

inline void RendererCommon::submitDrawCall(IDrawCall& drawCall, Entity& entity, const Mat44f& worldTransform)
{
	if(!mCurrentMaterial)
		return;

	RenderItem r = { &entity, &drawCall, mCurrentMaterial, worldTransform };

	Vec3f pos = worldTransform.translation();
	mViewMatrix.transformPoint(pos);
	const float dist = pos.z;

	if(!mCurrentMaterial->isTransparent())
		mOpaqueQueue.insert(*new RenderItemNode(-dist, r));
	else
		mTransparentQueue.insert(*new RenderItemNode(dist, r));
}

inline void RendererCommon::preRenderMaterial(size_t pass, IMaterialComponent& mtl) {
	mtl.preRender(pass, this);
}

inline void RendererCommon::postRenderMaterial(size_t pass, IMaterialComponent& mtl) {
	mtl.postRender(pass, this);
}

inline void RendererCommon::resetStatistic()
{
	memset(&mStatistic, 0, sizeof(mStatistic));
}

}	// namespace MCD