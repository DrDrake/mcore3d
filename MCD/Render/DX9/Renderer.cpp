#include "Pch.h"
#include "../Renderer.h"
#include "../Camera.h"
#include "../Light.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../RenderWindow.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Entity/Entity.h"

#include <d3d9.h>
#include <D3DX9Shader.h>
#include <stack>

#include "Material_DX9.inc"

namespace MCD {

struct ShaderContext
{
	LPDIRECT3DVERTEXSHADER9 shader;
	LPD3DXCONSTANTTABLE constTable;
};	// ShaderContext

struct RenderItem
{
	MeshComponent2* mesh;
	MaterialComponent* material;
};	// RenderItem

typedef std::vector<RenderItem> RenderItems;

class RendererComponent::Impl
{
public:
	Impl();

	void render(Entity& entityTree, CameraComponent2* camera);

	//! mDefaultCamera and mCurrentCamera may differ from each other for instance when rendering using light's view
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;

	ShaderContext mCurrentVS, mCurrentPS;

	sal_notnull RendererComponent* mBackRef;

	std::stack<MaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mRenderQueue;
};	// Impl

RendererComponent::Impl::Impl()
{
	ZeroMemory(&mCurrentVS, sizeof(mCurrentVS));
	ZeroMemory(&mCurrentPS, sizeof(mCurrentPS));

	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
}

void RendererComponent::Impl::render(Entity& entityTree, CameraComponent2* camera)
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	{	// Apply camera
		if(!camera) camera = mDefaultCamera.get();
		mCurrentCamera = camera;
		if(!camera || !camera->entity())
			return;

		camera->frustum.computeProjection(mProjMatrix.getPtr());
		mViewMatrix = camera->entity()->worldTransform().inverse();
		mViewProjMatrix = mProjMatrix * mViewMatrix;

		// TODO: Is these usefull?
		device->SetTransform(D3DTS_VIEW, (D3DMATRIX*)mViewMatrix.getPtr());
		device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)mProjMatrix.getPtr());

		if(mCurrentVS.constTable) {
			D3DXHANDLE handle;
			handle = mCurrentVS.constTable->GetConstantByName(nullptr, "mcdViewProj");
			mCurrentVS.constTable->SetMatrix(device, handle, (D3DXMATRIX*)mViewProjMatrix.getPtr());
		}
	}

	// Apply view port
/*	D3DVIEWPORT9 view_port;
	view_port.X=0;
	view_port.Y=0;
	view_port.Width=800;
	view_port.Height=600;
	view_port.MinZ=0.0f;
	view_port.MaxZ=1.0f;
	device->SetViewport(&view_port);*/

	Entity* last = nullptr;

	// Traverse the Entity tree
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		Entity* e = itr.current();

		MaterialComponent* mtl = e->findComponent<MaterialComponent>();

		{	// Material stack manuipuation
			if(e->parent() == last) {
				// Moving down in the tree
			}
			else {
				// Moving up in the tree unless 'last' is a slibing of 'e'
				while(e->parent() != last->parent()) {
					mMaterialStack.pop();
					last = last->parent();
				}

				mMaterialStack.pop();
			}

			mtl = mtl ? mtl : mMaterialStack.top();
			mMaterialStack.push(mtl);
			last = e;
		}

		// Push light into the light list, if any
		if(LightComponent* light = e->findComponent<LightComponent>()) {
			mLights.push_back(light);
		}

		// Push mesh into render queue, if any
		if(MeshComponent2* mesh = e->findComponent<MeshComponent2>()) {
			RenderItem r = { mesh, mtl };
			mRenderQueue.push_back(r);
		}

		itr.next();
	}

	// Render the items in render queue
	MCD_FOREACH(const RenderItem& i, mRenderQueue) {
		mWorldMatrix = i.mesh->entity()->worldTransform();
		mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

		// Set lighting information to the shader
		i.material->preRender(0, this);
		i.mesh->mesh->drawFaceOnly();
		i.material->postRender(0, this);
	}

	mLights.clear();
	mRenderQueue.clear();
}

RendererComponent::RendererComponent()
	: mImpl(*new Impl)
{
	mImpl.mBackRef = this;
}

RendererComponent::~RendererComponent()
{
	delete &mImpl;
}

void RendererComponent::setDefaultCamera(CameraComponent2& camera) {
	mImpl.mDefaultCamera = &camera;
}

CameraComponent2* RendererComponent::defaultCamera() const {
	return mImpl.mDefaultCamera.get();
}

void RendererComponent::render(Entity& entityTree, CameraComponent2* camera) {
	mImpl.render(entityTree, camera);
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	mImpl.mVsConstTable->SetMatrix(
		device, mImpl.mConstantHandles.worldViewProj, (D3DXMATRIX*)renderer.mWorldViewProjMatrix.getPtr()
	);

	mImpl.mVsConstTable->SetMatrix(
		device, mImpl.mConstantHandles.world, (D3DXMATRIX*)renderer.mWorldMatrix.getPtr()
	);

	// To match the light data structure in the shader
	struct LightStruct
	{
		Vec3f position;
		ColorRGBAf color;
	} lightStruct;

	for(size_t i=0; i<4; ++i) {
		D3DXHANDLE hi = mImpl.mPsConstTable->GetConstantElement(mImpl.mConstantHandles.lights, i);

		if(i < renderer.mLights.size()) {
			LightComponent* light = renderer.mLights[i];
			lightStruct.position = light->entity()->worldTransform().translation();
			lightStruct.color = ColorRGBAf(light->color, 1);
		}
		else
			memset(&lightStruct, 0, sizeof(lightStruct));

		HRESULT result = mImpl.mVsConstTable->SetFloatArray(device, hi, (float*)&lightStruct, sizeof(lightStruct) / sizeof(float));
		result = result;
	}

	device->SetVertexShader(mImpl.mVs);
	device->SetPixelShader(mImpl.mPs);
}

void MaterialComponent::postRender(size_t pass, void* context)
{
}

}	// namespace MCD
