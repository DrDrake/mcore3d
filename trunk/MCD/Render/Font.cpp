#include "Pch.h"
#include "Font.h"
#include "Texture.h"
#include "../Core/Math/Vec4.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {

BmpFont::BmpFont(const Path& fileId)
	: Resource(fileId)
{
	memset(&charSet, 0, sizeof(charSet));
}

BmpFont::~BmpFont() {}

int BmpFont::findKerningOffset(uint16_t char1, uint16_t char2) const
{
	const uint32_t key = (uint32_t(char1) << 16) + char2;
	const Kerning::const_iterator i = kerning.find(key);
	return i == kerning.end() ? 0 : i->second;
}

typedef std::map<FixString, BmpFontMaterialComponent*> BmpFontMaterial;
static BmpFontMaterial bmpFontMaterial;

BmpFontMaterialComponent* getMaterial(const FixString& fontResource)
{
	BmpFontMaterial::const_iterator i = bmpFontMaterial.find(fontResource);
	if(i != bmpFontMaterial.end()) {
		intrusivePtrAddRef(i->second);
		return i->second;
	}

	ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot();
	MCD_ASSUME(c);

	BmpFontPtr bmpFont = dynamic_cast<BmpFont*>(c->resourceManager().load(fontResource.c_str()).get());
	if(!bmpFont) return nullptr;

	BmpFontMaterialComponent* mtlCom = new BmpFontMaterialComponent;
	mtlCom->bmpFont = bmpFont;

	bmpFontMaterial[fontResource] = mtlCom;
	intrusivePtrAddRef(mtlCom);
	return mtlCom;
}

void releaseMaterial(const FixString& fontResource)
{
	if(fontResource.empty()) return;
	BmpFontMaterial::const_iterator i = bmpFontMaterial.find(fontResource);
	MCD_ASSERT(i != bmpFontMaterial.end());
	intrusivePtrRelease(i->second);
}

TextLabelComponent::TextLabelComponent()
	: lineWidth(0), mFontMaterial(nullptr)
	, mLastBmpFontCommitCount(0)
	, color(ColorRGBAf(1, 1))
{}

TextLabelComponent::~TextLabelComponent()
{
	releaseMaterial(font);
}

Component* TextLabelComponent::clone() const
{
	TextLabelComponent* ret = new TextLabelComponent;
	ret->text = text;
	ret->lineWidth = lineWidth;
	ret->color = color;
	ret->font = font;
	return ret;
}

// Example that handle text alignment and more:
// http://www.angelcode.com/dev/bmfonts/
// http://www.chadvernon.com/blog/resources/managed-directx-2/bitmap-fonts/
void TextLabelComponent::buildVertexBuffer(const BmpFont& font)
{
	mVertexBuffer.clear();
	Vec3f charPos = Vec3f::cZero;
	unsigned char lastCharacter = 0;

	// Construct the vertex buffer character by character
	for(size_t i=0; i<text.size(); ++i) {
		const unsigned char c = text.c_str()[i];
		const BmpFont::CharDescriptor& desc = font.charSet.chars[c];

		const int kerningOffset = font.findKerningOffset(lastCharacter, c);
		charPos.x += kerningOffset;

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
		lastCharacter = c;

		// New line handling
		if(c == '\n') {
			charPos.x = 0;
			charPos.y -= font.charSet.lineHeight;
		}
	}
}

void TextLabelComponent::update()
{
	if(!mFontMaterial)
		mFontMaterial = getMaterial(font);

	if(!mFontMaterial) return;

	BmpFont* bmpFont = mFontMaterial->bmpFont.get();
	MCD_ASSUME(bmpFont);

	const bool fontChanged = font != mLastFont || bmpFont->commitCount() != mLastBmpFontCommitCount;
	const bool textChanged = text != mLastText;

	if(font != mLastFont)
		releaseMaterial(mLastFont);

	if(fontChanged || textChanged)
	{
		buildVertexBuffer(*bmpFont);
		mLastText = text;
		mLastFont = font;
		mLastBmpFontCommitCount = bmpFont->commitCount();
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

}	// namespace MCD
