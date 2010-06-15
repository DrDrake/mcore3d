#ifndef __MCD_RENDER_RENDERTARGETCOMPONENT__
#define __MCD_RENDER_RENDERTARGETCOMPONENT__

#include "Renderable.h"

namespace MCD {

class RenderWindow;
typedef IntrusiveWeakPtr<class Entity> EntityPtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RendererComponent> RendererComponentPtr;

class MCD_RENDER_API RenderTargetComponent : public RenderableComponent2
{
public:
	RenderTargetComponent();
	sal_override ~RenderTargetComponent();

	//! Will register the itself into the RendererComponent
	sal_override void render() = 0;

	/*!	Will invoked by Renderer, preform some preparation and then calling
		back Renderer::render().
	 */
	virtual void render(RendererComponent& renderer) = 0;

	EntityPtr entityToRender;
	CameraComponent2Ptr cameraComponent;
	RendererComponentPtr rendererComponent;
};	// RenderTargetComponent

typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

class MCD_RENDER_API WindowRenderTargetComponent : public RenderTargetComponent
{
public:
	WindowRenderTargetComponent();

	sal_override void render();

	//!	Will invoke Renderer::render
	sal_override void render(RendererComponent& renderer);

	RenderWindow* window;
};	// WindowRenderTargetComponent

typedef IntrusivePtr<class Texture> TexturePtr;

class MCD_RENDER_API TextureRenderTargetComponent : public RenderTargetComponent
{
public:
	TextureRenderTargetComponent();
	sal_override ~TextureRenderTargetComponent();

	sal_override void render();

	//!	Will invoke Renderer::render
	sal_override void render(RendererComponent& renderer);

	//!	Creates a texture that suitable for use as render target
	TexturePtr createTexture(size_t width, size_t height);

	TexturePtr colorTexture;
	TexturePtr depthTexture;
};	// TextureRenderTargetComponent

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERTARGETCOMPONENT__
