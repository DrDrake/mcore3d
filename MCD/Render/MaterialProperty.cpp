#include "Pch.h"
#include "MaterialProperty.h"
#include "ShaderProgram.h"
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
	: unit(unit_), texture(texture_)
{
}

TextureProperty::TextureProperty(const TextureProperty& rhs)
	: texture(rhs.texture), unit(rhs.unit), shaderName(rhs.shaderName)
{
}

TextureProperty& TextureProperty::operator=(const TextureProperty& rhs)
{
	unit = rhs.unit;
	texture = rhs.texture;
	shaderName = rhs.shaderName;
	return *this;
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

// Shader
ShaderProperty::ShaderProperty(ShaderProgram* shaderProgram_)
	: shaderProgram(shaderProgram_)
{
}

ShaderProperty::ShaderProperty(const SharedPtr<ShaderProgram>& shaderProgram_)
	: shaderProgram(shaderProgram_)
{
}

ShaderProperty::ShaderProperty(const ShaderProperty& rhs)
	: shaderProgram(rhs.shaderProgram)
{}

ShaderProperty& ShaderProperty::operator=(const ShaderProperty& rhs)
{
	shaderProgram = rhs.shaderProgram;
	return *this;
}

ShaderProperty::~ShaderProperty()
{}

IMaterialProperty* ShaderProperty::clone() const
{
	return new ShaderProperty(*this);
}

void ShaderProperty::begin() const
{
	if(shaderProgram)
		shaderProgram->bind();
}

void ShaderProperty::end() const
{
	if(shaderProgram)
		shaderProgram->unbind();
}

// Font culling
void FrontCullingProperty::begin() const
{
	glCullFace(GL_FRONT);
}

void FrontCullingProperty::end() const
{
	glCullFace(GL_BACK);
}

// Enable state
EnableStateProperty::EnableStateProperty(int s)
	: state(s)
{}

void EnableStateProperty::begin() const
{
	glEnable(state);
}

void EnableStateProperty::end() const
{
	glDisable(state);
}

// Disable state
DisableStateProperty::DisableStateProperty(int s)
	: state(s)
{}

void DisableStateProperty::begin() const
{
	glDisable(state);
}

void DisableStateProperty::end() const
{
	glEnable(state);
}

// Line drawing
void LineDrawingProperty::begin() const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void LineDrawingProperty::end() const
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

LineWidthProperty::LineWidthProperty(float w)
	: width(w)
{}

void LineWidthProperty::begin() const
{
	glLineWidth(width);
}

void LineWidthProperty::end() const
{
	glLineWidth(1.0f);
}

// Blend Enable
BlendingProperty::BlendingProperty()
	: 
	blendEnable(false), sfactor(GL_ONE), dfactor(GL_ONE), blendEquation(GL_FUNC_ADD),
	blendEnableSep(false), sfactorSep(GL_ONE), dfactorSep(GL_ONE), blendEquationSep(GL_FUNC_ADD)
{
}

void BlendingProperty::begin() const
{
	if(blendEnable)
	{
		glEnable(GL_BLEND);

		if(blendEnableSep)
		{
			glBlendEquationSeparate(blendEquation, blendEquationSep);
			glBlendFuncSeparate(sfactor, dfactor, sfactorSep, dfactorSep);
		}
		else
		{
			glBlendEquation(blendEquation);
			glBlendFunc(sfactor, dfactor);
		}
	}
	else
	{
		glDisable(GL_BLEND);
	}

}

void BlendingProperty::end() const
{
	glDisable(GL_BLEND);
}

}	// namespace MCD
