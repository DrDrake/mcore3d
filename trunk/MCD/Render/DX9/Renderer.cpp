#include "Pch.h"
#include "Renderer.inc"
#include "Material_DX9.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Mesh.h"
#include "../RenderWindow.h"
#include "../../Core/Entity/Entity.h"
#include <D3DX9Shader.h>

namespace MCD {

RendererComponent::Impl::Impl()
{
	ZeroMemory(&mCurrentVS, sizeof(mCurrentVS));
	ZeroMemory(&mCurrentPS, sizeof(mCurrentPS));

	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	mWhiteTexture = new Texture("1x1White");
	byte_t data[] = { 255, 255, 255, 255 };
	mWhiteTexture->create(GpuDataFormat::get("uintARGB8"), GpuDataFormat::get("uintARGB8"), 1, 1, 1, 1, data, 4);
}

void RendererComponent::Impl::render(Entity& entityTree, CameraComponent2* camera)
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	{	// Apply camera
		if(!camera) camera = mDefaultCamera.get();
		mCurrentCamera = camera;
		if(!camera) return;
		Entity* cameraEntity = camera->entity();
		if(!cameraEntity) return;

		camera->frustum.computeProjection(mProjMatrix.getPtr());
		mCameraTransform = cameraEntity->worldTransform();
		mViewMatrix = mCameraTransform.inverse();
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

		mUniqueMaterial.insert(mtl);

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

	// Bind constants for all unique materials
//	MCD_FOREACH(MaterialComponent* m, mUniqueMaterial)
//		m->preRender(0, this);

	// Render the items in render queue
	MCD_FOREACH(const RenderItem& i, mRenderQueue) {
		if(Entity* e = i.mesh->entity()) {
			mWorldMatrix = e->worldTransform();
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			// Set lighting information to the shader
			i.material->preRender(0, this);
//			i.material->mImpl.updateWorldTransform(this);
			i.mesh->mesh->drawFaceOnly();
			i.material->postRender(0, this);
		}
	}

	mLights.clear();
	mRenderQueue.clear();
	mUniqueMaterial.clear();
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

}	// namespace MCD
