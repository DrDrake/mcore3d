#include "Pch.h"
#include "Renderer.inc"
#include "Material_DX9.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Mesh.h"
#include "../RenderTargetComponent.h"
#include "../RenderWindow.h"
#include "../../Core/Entity/Entity.h"
#include <D3DX9Shader.h>

namespace MCD {

struct Context {
	sal_maybenull LPDIRECT3DDEVICE9 mDevice;
	sal_maybenull LPDIRECT3DSWAPCHAIN9 mSwapChain;
};

LPDIRECT3DDEVICE9 getDevice()
{
	Context* context = reinterpret_cast<Context*>(RenderWindow::getActiveContext());
	return context ? context->mDevice : nullptr;
}

LPDIRECT3DSWAPCHAIN9 currentSwapChain()
{
	Context* context = reinterpret_cast<Context*>(RenderWindow::getActiveContext());
	return context ? context->mSwapChain : nullptr;
}

RendererComponent::Impl::Impl()
{
	ZeroMemory(&mCurrentVS, sizeof(mCurrentVS));
	ZeroMemory(&mCurrentPS, sizeof(mCurrentPS));

	LPDIRECT3DDEVICE9 device = getDevice();
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	mWhiteTexture = new Texture("1x1White");
	byte_t data[] = { 255, 255, 255, 255 };
	mWhiteTexture->create(GpuDataFormat::get("uintARGB8"), GpuDataFormat::get("uintARGB8"), 1, 1, 1, 1, data, 4);
}

RendererComponent::Impl::~Impl()
{
}

void RendererComponent::Impl::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	LPDIRECT3DDEVICE9 device = getDevice();

	{	// Apply camera
		CameraComponent2Ptr camera = renderTarget.cameraComponent;
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

	// Traverse the Entity tree
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		Entity* e = itr.current();

		// Push light into the light list, if any
		if(LightComponent* light = e->findComponent<LightComponent>()) {
			mLights.push_back(light);
		}

		// Pop material when moving up (towards parent) or leveling in the tree
		for(int depth = itr.depthChange(); depth <= 0 && mMaterialStack.size() > 0; ++depth)
			mMaterialStack.pop();

		// Push material
		MaterialComponent* mtl = e->findComponent<MaterialComponent>();
		if(nullptr == mtl) {
			// Skip if there where no material
			if(mMaterialStack.empty()) {
				itr.next();
				continue;
			}
			mtl = mMaterialStack.top();
		}
		mMaterialStack.push(mtl);

		// Push mesh into render queue, if any
		if(MeshComponent2* mesh = e->findComponent<MeshComponent2>()) {
			Vec3f pos = e->worldTransform().translation();
			mViewMatrix.transformPoint(pos);
			const float dist = pos.z;

			RenderItem r = { mesh, mtl };
			if(!mtl->isTransparent())
				mOpaqueQueue.insert(*new RenderItemNode(-dist, r));
			else
				mTransparentQueue.insert(*new RenderItemNode(dist, r));
		}

		itr.next();
	}

	{	// Render opaque items
		processRenderItems(mOpaqueQueue);
	}

	{	// Render transparent items
		// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=563635
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		processRenderItems(mTransparentQueue);

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}

	mLights.clear();
	mOpaqueQueue.destroyAll();
	mTransparentQueue.destroyAll();
}

void RendererComponent::Impl::processRenderItems(RenderItems& items)
{
	for(RenderItemNode* node = items.findMin(); node != nullptr; node = node->next()) {
		const RenderItem& i = node->mRenderItem;

		if(Entity* e = i.mesh->entity()) {
			mWorldMatrix = e->worldTransform();
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			// Set lighting information to the shader
			i.material->preRender(0, this);
			i.mesh->mesh->drawFaceOnly();
			i.material->postRender(0, this);
		}
	}
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

void RendererComponent::render(Entity& entityTree)
{
	// TODO: Update only RenderTargetComponent, to bring them into mImpl.mRenderTargets
	RenderableComponent2::traverseEntities(&entityTree);

	// Process the render targets one by one
	MCD_FOREACH(RenderTargetComponentPtr renderTarget, mImpl.mRenderTargets) {
		if(renderTarget) {
//			RenderableComponent2::traverseEntities(renderTarget->entityToRender.get());
			renderTarget->render(*this);
		}
	}
	mImpl.mRenderTargets.clear();
}

void RendererComponent::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	mImpl.render(entityTree, renderTarget);
}

}	// namespace MCD
