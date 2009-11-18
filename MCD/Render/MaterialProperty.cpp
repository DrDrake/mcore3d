#include "Pch.h"
#include "MaterialProperty.h"
#include "../../3Party/glew/glew.h"

/*! opengl api reference:
	http://pyopengl.sourceforge.net/documentation/manual-3.0/index.xhtml
*/

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
TextureProperty::TextureProperty(Texture* texture_, int unit_, int minFilter_, int magFilter_)
	: unit(unit_), texture(texture_), minFilter(minFilter_), magFilter(magFilter_)
{
}


IMaterialProperty* TextureProperty::clone() const
{
	return new TextureProperty(*this);
}

void TextureProperty::begin() const
{
	if(!texture || !texture->isValid())
		return;
	
	glActiveTexture(GL_TEXTURE0 + unit);

	glEnable(texture->type);
	texture->bind();
	
	glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, magFilter);
	
	glActiveTexture(GL_TEXTURE0);
}

void TextureProperty::end() const
{
	if(!texture || !texture->isValid())
		return;

	glActiveTexture(GL_TEXTURE0 + unit);
	texture->unbind();
	glDisable(texture->type);
	glActiveTexture(GL_TEXTURE0);
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

// Blending
BlendingProperty::BlendingProperty()
	: blendEnable(false), sfactor(GL_ONE), dfactor(GL_ONE), blendEquation(GL_FUNC_ADD)
	, blendEnableSep(false), sfactorSep(GL_ONE), dfactorSep(GL_ONE), blendEquationSep(GL_FUNC_ADD)
	, blendColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}

void BlendingProperty::begin() const
{
	if(blendEnable)
	{
		glEnable(GL_BLEND);

		glBlendColor(blendColor.r, blendColor.g, blendColor.b, blendColor.a);

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

// DepthStencil
DepthStencilProperty::DepthStencilProperty()
	: depthTestEnable(true), depthWriteEnable(true)
	, stencilTestEnable(false), depthFunc(GL_LEQUAL)
{
}

void DepthStencilProperty::begin() const
{
	// depth
	if(depthTestEnable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glDepthMask(depthWriteEnable ? GL_TRUE : GL_FALSE);
	glDepthFunc(depthFunc);

	// stencil
	if(stencilTestEnable)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void DepthStencilProperty::end() const
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glDisable(GL_STENCIL_TEST);
}

}	// namespace MCD
