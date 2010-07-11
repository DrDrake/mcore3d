#include "Pch.h"
#include "Font.h"
#include "Texture.h"

namespace MCD {

BmpFont::BmpFont(const Path& fileId)
	: Resource(fileId)
{
	memset(&charSet, 0, sizeof(charSet));
}

BmpFont::~BmpFont() {}

FontComponent::FontComponent()
	: lineWidth(0)
{}

FontComponent::~FontComponent() {}

bool FontComponent::cloneable() const { return true; }

Component* FontComponent::clone() const
{
	FontComponent* ret = new FontComponent;
	ret->text = text;
	ret->lineWidth = lineWidth;
	ret->bmpFont = bmpFont;
	return ret;
}

}	// namespace MCD
