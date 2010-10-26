#include "Pch.h"
#include "Renderer.inc"
#include "Material_DX9.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Mesh.h"
#include "../RenderTargetComponent.h"
#include "../RenderWindow.h"
#include <D3DX9Shader.h>

namespace MCD {

struct Context
{
	HWND wnd;
	size_t width, height;
	sal_maybenull LPDIRECT3DDEVICE9 mDevice;
	sal_maybenull LPDIRECT3DSWAPCHAIN9 mSwapChain;
};	// Context

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

	mWhiteTexture = new Texture("1x1White");
	byte_t data[] = { 255, 255, 255, 255 };
	MCD_VERIFY(mWhiteTexture->create(GpuDataFormat::get("uintARGB8"), GpuDataFormat::get("uintARGB8"), 1, 1, 1, 1, data, 4));

	resetStatistic();
}

RendererComponent::Impl::~Impl()
{
}

void RendererComponent::Impl::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	LPDIRECT3DDEVICE9 device = getDevice();

	{	// Apply camera
		CameraComponentPtr camera = renderTarget.cameraComponent;
		if(!camera) return;
		Entity* cameraEntity = camera->entity();
		if(!cameraEntity) return;

		camera->frustum.computeProjection(mProjMatrix.getPtr());
		mCameraTransform = cameraEntity->worldTransform();
		mViewMatrix = mCameraTransform.inverse();
		mViewProjMatrix = mProjMatrix * mViewMatrix;

		// Fixed pipeline will need these
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
	traverseEntities(entityTree);

	{	// Disable lighting if there is no LightComponent
		device->SetRenderState(D3DRS_LIGHTING, !mLights.empty());
	}

	{	// Render opaque items
		processRenderItems(mOpaqueQueue, mStatistic.opaque, mStatistic.materialSwitch);
	}

	{	// Render transparent items
		// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=563635
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		processRenderItems(mTransparentQueue, mStatistic.transparent, mStatistic.materialSwitch);

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}

	// Reset the last material
	if(mLastMaterial) {
		mLastMaterial->postRender(0, this);
		mLastMaterial = nullptr;
	}

	mLights.clear();
	mOpaqueQueue.destroyAll();
	mTransparentQueue.destroyAll();
}

void RendererComponent::Impl::render(Entity& entityTree)
{
	// Process the render targets one by one
	for(size_t i=0; i<mRenderTargets.size(); ++i) {
		if(RenderTargetComponent* r1 = mRenderTargets[i].get()) {
			const RenderWindow* w1 = r1 ? r1->window : nullptr;
			const RenderTargetComponent* r2 = (i+1 == mRenderTargets.size()) ? nullptr : mRenderTargets[i+1].get();
			const RenderWindow* w2 = r2 ? r2->window : nullptr;
			const bool swapBuffers = (w1 != w2);
			r1->render(*mBackRef, swapBuffers);
		}
	}
	mRenderTargets.clear();
}

void RendererComponent::Impl::processRenderItems(RenderItems& items, IDrawCall::Statistic& statistic, size_t& materialSwitch)
{
	for(RenderItemNode* node = items.findMin(); node != nullptr; node = node->next())
	{
		const RenderItem& i = node->mRenderItem;

		if(Entity* e = i.entity) {
			mWorldMatrix = i.worldTransform;
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			IMaterialComponent* mtl = i.material;
			MCD_ASSUME(mtl);

			if(mtl != mLastMaterial)
				++materialSwitch;

			// The material class will preform early out if mtl == mLastMaterial
			// NOTE: Must call for every RenderItem because the material is also
			// responsible for setting up the various matrix shader constants.
			mtl->preRender(0, this);
			i.drawCall->draw(this, statistic);
			mtl->postRender(0, this);
			mLastMaterial = mtl;
		}
	}
}

struct QuadVertex
{
	D3DXVECTOR3 position;
	float u, v;
};	// QuadVertex

static const int cQuadVertexFVF = (D3DFVF_XYZ|D3DFVF_TEX1);

void QuadRenderer::push(const Mat44f& transform, float width, float height, const Vec4f& uv, IMaterialComponent* mtl)
{
	// Trigger a flush whenever the material is changed or too many quad in the buffer
	if(mtl != mCurrentMaterial || mQuadCount > 512)
		flush();
	mCurrentMaterial = mtl;

	const float halfW = 0.5f * width;
	const float halfH = 0.5f * height;

	++mQuadCount;
	Vertex v[4] = {
		{ Vec3f(-halfW,  halfH, 0), Vec2f(uv.x, uv.y) },
		{ Vec3f(-halfW, -halfH, 0), Vec2f(uv.x, uv.w) },
		{ Vec3f( halfW, -halfH, 0), Vec2f(uv.z, uv.w) },
		{ Vec3f( halfW,  halfH, 0), Vec2f(uv.z, uv.y) },
	};

	for(size_t i=0; i<MCD_COUNTOF(v); ++i)
		transform.transformPoint(v[i].position);

	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[1]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[3]);
}

void QuadRenderer::flush()
{
	MCD_ASSERT(mVertexBuffer.size() == mQuadCount * 6);
	if(!mQuadCount)
		return;

	mRenderer.preRenderMaterial(0, *mCurrentMaterial);

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	device->SetFVF(cQuadVertexFVF);
	device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mQuadCount * 2, &mVertexBuffer[0], sizeof(QuadVertex));

	mVertexBuffer.clear();
	mQuadCount = 0;

	mRenderer.postRenderMaterial(0, *mCurrentMaterial);
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
	mImpl.render(entityTree);
}

void RendererComponent::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	mImpl.render(entityTree, renderTarget);
}

static RendererComponent* gCurrentRendererComponent = nullptr;

RendererComponent& RendererComponent::current()
{
	MCD_ASSUME(gCurrentRendererComponent);
	return *gCurrentRendererComponent;
}

void RendererComponent::begin()
{
	gCurrentRendererComponent = this;
	mImpl.resetStatistic();
}

void RendererComponent::end(float dt)
{
	gCurrentRendererComponent = nullptr;
}

}	// namespace MCD
