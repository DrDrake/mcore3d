#include "Pch.h"
#include "MaterialProperty.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

ColorProperty::ColorProperty(const ColorRGBAf& color, ColorOperation::Enum operation)
	: mColor(color), mOperation(operation)
{
}

void ColorProperty::apply(ColorRGBAf& dest) const
{
	switch(mOperation) {
		case ColorOperation::Replace :	dest = mColor;	break;
//		case ColorOperation::Add :		dest += mColor;	break;
//		case ColorOperation::Multiply :	dest *= mColor;	break;
		default: MCD_ASSERT(false);
	}
}

// Ambient
ColorRGBAf AmbientProperty::mCurrent(0, 0);

void AmbientProperty::begin() const
{
	apply(mCurrent);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mCurrent.rawPointer());
}

// Diffuse
ColorRGBAf DiffuseProperty::mCurrent(0, 0);

void DiffuseProperty::begin() const
{
	apply(mCurrent);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mCurrent.rawPointer());
}

// Specular
ColorRGBAf SpecularProperty::mCurrent(0, 0);

void SpecularProperty::begin() const
{
	apply(mCurrent);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mCurrent.rawPointer());
}

// Shininess
void ShininessProperty::begin() const
{
	glMaterialf(GL_FRONT, GL_SHININESS, mValue);
}

// Standard
StandardProperty::StandardProperty(
	const ColorRGBAf& ambient,
	const ColorRGBAf& diffuse,
	const ColorRGBAf& specular,
	ColorProperty::ColorOperation::Enum operation,
	float shininess
)
	:
	mAmbient(AmbientProperty(ambient, operation)),
	mDiffuse(DiffuseProperty(diffuse, operation)),
	mSpecular(SpecularProperty(specular, operation)),
	mShininess(shininess)
{
}

void StandardProperty::begin() const
{
	mAmbient.begin();
	mDiffuse.begin();
	mSpecular.begin();
	mShininess.begin();
}

// Texture
TextureProperty::TextureProperty(Texture* texture_, int unit_)
	: texture(texture_), unit(unit_)
{
}

TextureProperty::~TextureProperty()
{}

IMaterialProperty* TextureProperty::clone() const
{
	return new TextureProperty(*this);
}

void TextureProperty::begin() const
{
	if(!texture)
		return;

	if(unit == 0)
		texture->bind();
	else
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		texture->bind();
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
}

void TextureProperty::end() const
{
	if(texture)
		texture->unbind();

	if(!texture || unit <= 0)
		return;

	glActiveTextureARB(GL_TEXTURE0_ARB + unit);
	glDisable(texture->type);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void LineDrawingProperty::begin() const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void LineDrawingProperty::end() const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

}	// namespace MCD
