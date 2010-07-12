#ifndef __MCD_RENDER_FONT__
#define __MCD_RENDER_FONT__

#include "Renderable.h"
#include "../Core/System/Resource.h"

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
class MCD_RENDER_API FontComponent : public RenderableComponent2
{
public:
	FontComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const;
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();
	sal_override void render2(sal_in void* context);

// Attributes
	std::string text;
	size_t lineWidth;	//!< The maximum pixel for a line of text, word longer than that will move to next line. Zero means no limit
	BmpFontPtr bmpFont;

protected:
	sal_override ~FontComponent();
	ComponentPtr dummy;	// NOTE: Workaround for VC9 LNK1194 with vftable
};	// FontComponent

typedef IntrusiveWeakPtr<FontComponent> FontComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_FONT__
