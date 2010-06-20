#ifndef __MCD_RENDER_RENDERTARGETCOMPONENT__
#define __MCD_RENDER_RENDERTARGETCOMPONENT__

#include "Color.h"
#include "Renderable.h"
#include "../Core/System/Array.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

struct GpuDataFormat;
class RenderWindow;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class Entity> EntityPtr;
typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RendererComponent> RendererComponentPtr;

/*!	
 */
class MCD_RENDER_API RenderTargetComponent : public RenderableComponent2
{
public:
	RenderTargetComponent();

	~RenderTargetComponent();

	//! Will register the itself into the RendererComponent
	sal_override void render();

	/*!	Will invoked by Renderer, preform some preparation and then calling
		back Renderer::render().
	 */
	void render(RendererComponent& renderer);

	//!	Creates a texture that suitable for use as render target
	TexturePtr createTexture(const GpuDataFormat& format, size_t width, size_t height);

// Attributes
	EntityPtr entityToRender;
	CameraComponent2Ptr cameraComponent;
	RendererComponentPtr rendererComponent;

	ColorRGBf clearColor;
	Vec2<size_t> viewPortLeftTop;
	Vec2<size_t> viewPortWidthHeight;	//!< Zero means use full size of the target.

	sal_maybenull RenderWindow* window;
	Array<TexturePtr, 4> textures;

protected:
	int mImpl;
};	// RenderTargetComponent

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERTARGETCOMPONENT__
