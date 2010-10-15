#ifndef __MCD_RENDER_SPRITE__
#define __MCD_RENDER_SPRITE__

#include "Renderable.h"

namespace MCD {

class MCD_RENDER_API SpriteManagerComponent : protected DisplayListComponent
{
public:
// Operations

// Attributes
	std::vector<TexturePtr> textures;

protected:
	sal_override void render(sal_in void* context);
};	// SpriteManagerComponent

typedef IntrusiveWeakPtr<SpriteManagerComponent> SpriteManagerComponentPtr;

/*!
	The unit for dimensional data depends on the camera used.
	To have pixel-wised unit, make a Ortho camera with width/height matching the screen size.
 */
class MCD_RENDER_API SpriteComponent : public RenderableComponent
{
public:
	sal_override ~SpriteComponent();

// Operations

// Attributes
	ColorRGBAf color;
	size_t textureIndex;
	float width, height;

	SpriteManagerComponentPtr spriteManager;

	// Possible animation semantics:
	// x y z r		position and rotation
	// r g b a		color
	// u v sx sy	uv and scale

protected:
	sal_override void render(sal_in void* context);
};	// SpriteComponent

typedef IntrusiveWeakPtr<SpriteComponent> SpriteComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_SPRITE__
