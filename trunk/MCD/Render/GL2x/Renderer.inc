#include "../Renderer.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/System/Deque.h"
#include "../../Core/System/Map.h"
#include <vector>

namespace MCD {

class LightComponent;
class MaterialComponent;
class MeshComponent2;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

struct RenderItem
{
	sal_notnull MeshComponent2* mesh;
	sal_notnull MaterialComponent* material;
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
	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void processRenderItems(RenderItems& items);

	//! mDefaultCamera and mCurrentCamera may differ from each other when rendering using light's view
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;
	sal_notnull RendererComponent* mBackRef;
	std::stack<MaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mTransparentQueue, mOpaqueQueue;

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;
};	// Impl

}	// namespace MCD
