#include "../Renderer.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/System/Deque.h"
#include "../../Core/System/Map.h"
#include <vector>

typedef struct IDirect3DDevice9 *LPDIRECT3DDEVICE9;
typedef struct IDirect3DSwapChain9 *LPDIRECT3DSWAPCHAIN9;
typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9;
struct ID3DXConstantTable;
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

namespace MCD {

class LightComponent;
class MaterialComponent;
class MeshComponent2;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

template<typename T> void SAFE_RELEASE(T& p)
{
	if(p) {
		p->Release();
		p = nullptr;
	}
}

struct ShaderContext
{
	LPDIRECT3DVERTEXSHADER9 shader;
	LPD3DXCONSTANTTABLE constTable;
};	// ShaderContext

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

sal_maybenull MCD_RENDER_API LPDIRECT3DDEVICE9 getDevice();

sal_maybenull MCD_RENDER_API LPDIRECT3DSWAPCHAIN9 currentSwapChain();

class MCD_RENDER_API RendererComponent::Impl
{
public:
	Impl();

	~Impl();

	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void processRenderItems(RenderItems& items);

	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;

	ShaderContext mCurrentVS, mCurrentPS;

	sal_notnull RendererComponent* mBackRef;

	std::stack<MaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mTransparentQueue, mOpaqueQueue;

	TexturePtr mWhiteTexture;	//!< A 1x1 white texture, such that we alwasy feed the shader with texture

	typedef std::vector<RenderTargetComponentPtr> RenderTargets;
	RenderTargets mRenderTargets;
};	// Impl

}	// namespace MCD