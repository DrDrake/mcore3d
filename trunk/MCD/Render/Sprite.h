#ifndef __MCD_RENDER_SPRITE__
#define __MCD_RENDER_SPRITE__

#include "Animation.h"
#include "Color.h"
#include "Renderable.h"
#include "Texture.h"
#include "../Core/Math/Vec4.h"

namespace MCD {

/// A Sprite contains the necessary information for SpriteAtlas to render.
/// The unit for dimensional data depends on the camera used.
/// To have pixel-wised unit, make a Ortho camera with width/height matching the screen size.
class MCD_RENDER_API SpriteComponent : public AnimatedComponent
{
public:
	SpriteComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(SpriteComponent);
	}

// Operations

// Attributes
	ColorRGBAf color;

	/// The rectangle region to use in the texture atlas
	/// The four values are left, top, right and bottom.
	/// For values less than 2, it will be interpreted as conventional uv
	/// coordinate, otherwise will interpreted in unit of pixel.
	Vec4f textureRect;

	/// The rooting position of the sprite, in unit of relative size (ie 0.5,0.5 -> centre of the sprite)
	Vec2f anchor;

	float width, height;

	// Possible animation semantics:
	//  x  y  z  r		position and rotation
	// ax ay sx sy		anchor position and scaling
	//  r  g  b  a		color
	// rl  v  w  z		texture rectangle
	size_t trackOffset;	//!< Starting index to the animation's tracks
	AnimationComponentPtr animation;

	static const size_t trackPerSprite = 3;

protected:
	sal_override void update();
};	// SpriteComponent

typedef IntrusiveWeakPtr<SpriteComponent> SpriteComponentPtr;

/// To perform the actual rendering of SpriteComponent.
/// Data are feed by SpriteUpdaterComponent
class MCD_RENDER_API SpriteAtlasComponent : public RenderableComponent, public IDrawCall
{
public:
	SpriteAtlasComponent();

	sal_override ~SpriteAtlasComponent();

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

	class Impl;
	Impl* mImpl;
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
