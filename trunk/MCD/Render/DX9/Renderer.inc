#ifndef __MCD_RENDER_DX9_RENDERER__
#define __MCD_RENDER_DX9_RENDERER__

#include "../Renderer.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Math/Vec2.h"
#include "../../Core/Math/Vec3.h"
#include "../../Core/System/Array.h"
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
class IMaterialComponent;
class MeshComponent2;
class QuadComponent;
class QuadRenderer;
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
	sal_maybenull Entity* entity;
	sal_notnull IDrawCall* drawCall;
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

sal_maybenull MCD_RENDER_API LPDIRECT3DDEVICE9 getDevice();

sal_maybenull MCD_RENDER_API LPDIRECT3DSWAPCHAIN9 currentSwapChain();

class MCD_RENDER_API RendererComponent::Impl
{
public:
	Impl();

	~Impl();

	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void render(Entity& entityTree);

	void processRenderItems(RenderItems& items);

	void preRenderMaterial(size_t pass, IMaterialComponent& mtl);
	void postRenderMaterial(size_t pass, IMaterialComponent& mtl);

	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;

	ShaderContext mCurrentVS, mCurrentPS;

	sal_notnull RendererComponent* mBackRef;

	IMaterialComponent* mLastMaterial;	//!< To reduce material setup cost
	std::stack<IMaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mTransparentQueue, mOpaqueQueue;

	TexturePtr mWhiteTexture;	//!< A 1x1 white texture, such that we alwasy feed the shader with texture

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

#endif	// __MCD_RENDER_DX9_RENDERER__
