#include "../Renderer.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/System/Deque.h"
#include <set>
#include <vector>

typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9;
struct ID3DXConstantTable;
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

namespace MCD {

class LightComponent;
class MaterialComponent;
class MeshComponent2;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;

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

typedef std::vector<RenderItem> RenderItems;

class RendererComponent::Impl
{
public:
	Impl();

	void render(Entity& entityTree, CameraComponent2* camera);

	void processRenderItems(RenderItems& items);

	//! mDefaultCamera and mCurrentCamera may differ from each other for instance when rendering using light's view
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;

	ShaderContext mCurrentVS, mCurrentPS;

	sal_notnull RendererComponent* mBackRef;

	std::stack<MaterialComponent*> mMaterialStack;
	std::set<MaterialComponent*> mUniqueMaterial;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mTransparentQueue, mOpaqueQueue;

	TexturePtr mWhiteTexture;	//!< A 1x1 white texture, such that we alwasy feed the shader with texture
};	// Impl

}	// namespace MCD
