#include "Pch.h"
#include "../RenderTarget.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../RenderWindow.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"
#include <d3dx9.h>

namespace MCD {

RenderTargetComponent::RenderTargetComponent()
	: shouldClearColor(true), shouldClearDepth(true)
	, clearColor(0, 1)
	, viewPortLeftTop(0), viewPortWidthHeight(0)
	, window(nullptr)
{}

RenderTargetComponent::~RenderTargetComponent()
{}

void RenderTargetComponent::gather()
{
	RendererComponent& r = RendererComponent::current();
	r.mImpl.mRenderTargets.push_back(this);
}

TexturePtr RenderTargetComponent::createTexture(const GpuDataFormat& format, size_t width, size_t height)
{
	TexturePtr texture = new Texture("");
	if(!texture->create(format, format, width, height, 1, 1, nullptr, 0, D3DUSAGE_RENDERTARGET))
		texture = nullptr;
	return texture;
}

static void setViewPort(LPDIRECT3DDEVICE9 device, RenderTargetComponent& renderTarget)
{
	D3DVIEWPORT9 viewPort;
	viewPort.X = renderTarget.viewPortLeftTop.x;
	viewPort.Y = renderTarget.viewPortLeftTop.y;

	viewPort.Width =
		renderTarget.viewPortWidthHeight.x != 0 ?
			renderTarget.viewPortWidthHeight.x :
		renderTarget.window ?
			renderTarget.window->width() :
		renderTarget.textures[0] ?
			renderTarget.textures[0]->width :
			0;
	
	viewPort.Height =
		renderTarget.viewPortWidthHeight.y != 0 ?
			renderTarget.viewPortWidthHeight.y :
		renderTarget.window ?
			renderTarget.window->height() :
		renderTarget.textures[0] ?
			renderTarget.textures[0]->height :
			0;

	// Minimized window may resulting zero width height
	if(viewPort.Width * viewPort.Height == 0)
		return;

	viewPort.MinZ = 0;
	viewPort.MaxZ = 1;
	device->SetViewport(&viewPort);

	// Also adjust the frustum's aspect ratio
	if(renderTarget.cameraComponent && renderTarget.cameraComponent->frustum.projectionType == Frustum::Perspective)
		renderTarget.cameraComponent->frustum.setAcpectRatio(float(viewPort.Width) / viewPort.Height);
}

void RenderTargetComponent::render(RendererComponent& renderer)
{
	if(!entityToRender)
		return;

	size_t textureCount = 0;
	for(; textureCount<textures.size(); ++textureCount)
		if(!textures[textureCount]) break;

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	D3DCOLOR color = D3DCOLOR_RGBA(char(clearColor.r*255), char(clearColor.g*255), char(clearColor.b*255), char(clearColor.a*255));

	// Window only
	if(window && !textureCount) {
		window->makeActive();
		window->preUpdate();

		setViewPort(device, *this);
		if(shouldClearColor || shouldClearDepth) device->Clear(
			0, nullptr,
			shouldClearColor * D3DCLEAR_TARGET | shouldClearDepth * D3DCLEAR_ZBUFFER,
			color, 1.0f, 0
		);
		device->BeginScene();

		renderer.render(*entityToRender, *this);

		device->EndScene();
	}

	// Texture only
	// http://www.borgsoft.de/renderToSurface.html
	if(!window && textureCount) {
		for(size_t i=0; i<textureCount; ++i)
		{
			LPDIRECT3DSURFACE9 surface = nullptr;
			Texture* texture = textures[i].getNotNull();
			IDirect3DTexture9* tex = reinterpret_cast<IDirect3DTexture9*>(texture->handle);
			tex->GetSurfaceLevel(0, &surface);
			MCD_ASSUME(surface);
			device->SetRenderTarget(i, surface);
			SAFE_RELEASE(surface);
		}

		setViewPort(device, *this);
		if(shouldClearColor || shouldClearDepth) device->Clear(
			0, nullptr,
			shouldClearColor * D3DCLEAR_TARGET | shouldClearDepth * D3DCLEAR_ZBUFFER,
			color, 1.0f, 0
		);
		device->BeginScene();

		renderer.render(*entityToRender, *this);

		device->EndScene();
	}
}

size_t RenderTargetComponent::targetWidth() const
{
	if(window) return window->width();
	if(textures[0]) return textures[0]->width;
	return 0;
}

size_t RenderTargetComponent::targetHeight() const
{
	if(window) return window->height();
	if(textures[0]) return textures[0]->height;
	return 0;
}

}	// namespace MCD
