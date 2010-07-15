#ifndef __MCD_RENDER_FONT__
#define __MCD_RENDER_FONT__

#include "Material.h"
#include "../Core/System/Resource.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

typedef IntrusivePtr<class Texture> TexturePtr;

/*!	Store font information like character size, padding, spacing, kerning etc.
 */
class MCD_RENDER_API BmpFont : public Resource
{
public:
	explicit BmpFont(const Path& fileId);

	TexturePtr texture;

	struct CharDescriptor
	{	// Clean 16 bytes
		uint16_t x, y;				//!< Pixel position in the texture
		uint16_t width, height;		//!< Width and height of the character in pixels
		uint16_t xOffset, yOffset;	//!< The offset value for drawing
		uint16_t xAdvance;			//!< Amount to the next character
		uint16_t page;				//!< Which texture page to use if the characters were spreaded over a number of pages
	};	// CharDescriptor

	struct CharSet
	{
		uint16_t lineHeight;
		uint16_t base;
		uint16_t width, height;
		uint16_t pages;
		CharDescriptor chars[256];	// TODO: Supports more than 256
	};	// CharSet

	CharSet charSet;

protected:
	sal_override ~BmpFont();
};	// BmpFont

typedef IntrusivePtr<BmpFont> BmpFontPtr;

//!	The text is anchored at the left top corner
class MCD_RENDER_API BmpFontMaterialComponent : public IMaterialComponent
{
public:
	BmpFontMaterialComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();
	sal_override void render2(sal_in void* context);

// Attributes
	BmpFontPtr bmpFont;

	sal_override bool isTransparent() const { return true; }

protected:
	~BmpFontMaterialComponent() {}

	//!	Invoked by RendererComponent
	sal_override void preRender(size_t pass, void* context);
	sal_override void postRender(size_t pass, void* context);
};	// BmpFontMaterialComponent

//!	The text is anchored at the left top corner
class MCD_RENDER_API TextLabelComponent : public RenderableComponent2, public IDrawCall
{
public:
	TextLabelComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const;
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();
	sal_override void render2(sal_in void* context);
	sal_override void draw(sal_in void* context);

// Attributes
	std::string text;
	size_t lineWidth;	//!< The maximum pixel for a line of text, word longer than that will move to next line. Zero means no limit

protected:
	sal_override ~TextLabelComponent();
	void buildVertexBuffer(const BmpFont& font);

	ComponentPtr dummy;	// NOTE: Workaround for VC9 LNK1194 with vftable

	uint32_t mStringHash;	// To determine the text have been changed or not
	size_t mLastBmpFontCommitCount;	// To determine the font resource is loaded or not
	IntrusiveWeakPtr<const BmpFont> mLastBmpFont;	// To determine the font resource is changed or not

	// Caching of vertex buffer
	struct Vertex { Vec3f position; Vec2f uv; };
	std::vector<Vertex> mVertexBuffer;
};	// TextLabelComponent

typedef IntrusiveWeakPtr<TextLabelComponent> TextLabelComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_FONT__
