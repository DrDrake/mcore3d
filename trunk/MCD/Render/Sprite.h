#ifndef __MCD_RENDER_SPRITE__
#define __MCD_RENDER_SPRITE__

#include "Color.h"
#include "Renderable.h"
#include "Texture.h"
#include "../Core/Math/Vec4.h"

namespace MCD {

/// A Sprite contains the necessary information for SpriteAtlas to render.
/// The unit for dimensional data depends on the camera used.
/// To have pixel-wised unit, make a Ortho camera with width/height matching the screen size.
class MCD_RENDER_API SpriteComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(SpriteComponent);
	}

// Operations

// Attributes
	ColorRGBAf color;
	Vec4f uv;	/// <1 for conventional uv coordinate, >1 for pixel unit
	float width, height;

	// Possible animation semantics:
	// x y z r		position and rotation
	// r g b a		color
	// u v sx sy	uv and scale

protected:
	sal_override void gather();
};	// SpriteComponent

typedef IntrusiveWeakPtr<SpriteComponent> SpriteComponentPtr;

/// To perform the actual rendering of SpriteComponent.
/// Data are feed by SpriteUpdaterComponent
class MCD_RENDER_API SpriteAtlasComponent : public RenderableComponent, public IDrawCall
{
public:
// Operations

// Attributes
	TexturePtr textureAtlas;

protected:
	friend class SpriteComponent;
	friend class SpriteUpdaterComponent;
	void gatherSprite(SpriteComponent* sprite);
	sal_override void gather();
	sal_override void render(sal_in void* context);
	sal_override void draw(sal_in void* context, Statistic& statistic);

	struct Vertex
	{
		Vec3f position;
		Vec2f uv;
		ColorRGBAf color;
	};	// Vertex
	std::vector<Vertex> mVertexBuffer;
};	// SpriteAtlasComponent

typedef IntrusiveWeakPtr<SpriteAtlasComponent> SpriteAtlasComponentPtr;

/// Gather SpriteComponent and use them to fill the vertex data into their
/// corresponding SpriteAtlasComponent
class MCD_RENDER_API SpriteUpdaterComponent : public ComponentUpdater
{
public:
// Operations
	sal_override void begin();
	sal_override void end(float dt);

protected:
	friend class SpriteAtlasComponent;
	std::vector<SpriteAtlasComponent*> mSpriteAtlas;
};	// SpriteUpdaterComponent

typedef IntrusiveWeakPtr<SpriteUpdaterComponent> SpriteUpdaterComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_SPRITE__
