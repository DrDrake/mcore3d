#include "Pch.h"
#include "Font.h"
#include "Texture.h"
#include "../Core/Math/Vec4.h"

namespace MCD {

BmpFont::BmpFont(const Path& fileId)
	: Resource(fileId)
{
	memset(&charSet, 0, sizeof(charSet));
}

BmpFont::~BmpFont() {}

TextLabelComponent::TextLabelComponent()
	: lineWidth(0), mStringHash(0), mLastBmpFontCommitCount(0)
{}

TextLabelComponent::~TextLabelComponent() {}

Component* TextLabelComponent::clone() const
{
	TextLabelComponent* ret = new TextLabelComponent;
	ret->text = text;
	ret->lineWidth = lineWidth;
	return ret;
}

// Example that handle text alignment and more:
// http://www.angelcode.com/dev/bmfonts/
// http://www.chadvernon.com/blog/resources/managed-directx-2/bitmap-fonts/
void TextLabelComponent::buildVertexBuffer(const BmpFont& font)
{
	mVertexBuffer.clear();
	Vec3f charPos = Vec3f::cZero;

	// Construct the vertex buffer character by character
	for(std::string::const_iterator i=text.begin(); i != text.end(); ++i) {
		const unsigned char c = *i;
		const BmpFont::CharDescriptor& desc = font.charSet.chars[c];

		Vec4f uv(desc.x, desc.y, float(desc.x + desc.width), float(desc.y + desc.height));
		uv.x /= font.charSet.width;
		uv.y /= font.charSet.height;
		uv.z /= font.charSet.width;
		uv.w /= font.charSet.height;

		const float left = charPos.x + desc.xOffset;
		const float top = charPos.y - desc.yOffset;
		const float right = left + desc.width;
		const float bottom = top - desc.height;

		Vertex v[4] = {
			{	Vec3f(left, top, 0),		Vec2f(uv.x, uv.y)	},	// Left top
			{	Vec3f(left, bottom, 0),		Vec2f(uv.x, uv.w)	},	// Left bottom
			{	Vec3f(right, bottom, 0),	Vec2f(uv.z, uv.w)	},	// Right bottom
			{	Vec3f(right, top, 0),		Vec2f(uv.z, uv.y)	}	// Right top
		};

		mVertexBuffer.push_back(v[0]);
		mVertexBuffer.push_back(v[1]);
		mVertexBuffer.push_back(v[2]);
		mVertexBuffer.push_back(v[0]);
		mVertexBuffer.push_back(v[2]);
		mVertexBuffer.push_back(v[3]);

		charPos.x += desc.xAdvance;
		
		// New line handling
		if(c == '\n') {
			charPos.x = 0;
			charPos.y -= font.charSet.lineHeight;
		}
	}
}

BmpFontMaterialComponent::BmpFontMaterialComponent()
	: color(ColorRGBAf(1, 1))
{}

Component* BmpFontMaterialComponent::clone() const
{
	BmpFontMaterialComponent* ret = new BmpFontMaterialComponent;
	ret->bmpFont = bmpFont;
	ret->color = color;
	return ret;
}

void BmpFontMaterialComponent::render() {}

}	// namespace MCD
