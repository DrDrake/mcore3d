#ifndef __MCD_RENDER_DX9_RENDERER__
#define __MCD_RENDER_DX9_RENDERER__

#include "../Renderer.h"
#include "../Renderer.inc"
#include "../../Core/Math/Vec2.h"
#include "../../Core/Math/Vec3.h"
#include "../../Core/System/Array.h"
#include <vector>

typedef struct IDirect3DDevice9 *LPDIRECT3DDEVICE9;
typedef struct IDirect3DSwapChain9 *LPDIRECT3DSWAPCHAIN9;
typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9;
struct ID3DXConstantTable;
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

namespace MCD {

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

sal_maybenull MCD_RENDER_API LPDIRECT3DDEVICE9 getDevice();

sal_maybenull MCD_RENDER_API LPDIRECT3DSWAPCHAIN9 currentSwapChain();

class MCD_RENDER_API RendererComponent::Impl : public RendererCommon
{
public:
	Impl();

	~Impl();

	void render(Entity& entityTree, RenderTargetComponent& renderTarget);

	void render(Entity& entityTree);

	void processRenderItems(RenderItems& items, IDrawCall::Statistic& statistic, size_t& materialSwitch);

	ShaderContext mCurrentVS, mCurrentPS;

	TexturePtr mBlackTexture;	//!< A 1x1 black texture, such that we always feed the shader with texture
	TexturePtr mWhiteTexture;	//!< A 1x1 white texture, such that we always feed the shader with texture
};	// Impl

}	// namespace MCD

#endif	// __MCD_RENDER_DX9_RENDERER__
