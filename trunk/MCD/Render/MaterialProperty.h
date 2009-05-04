#ifndef __MCD_RENDER_MATERIALPROPERTY__
#define __MCD_RENDER_MATERIALPROPERTY__

#include "ShareLib.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class MCD_RENDER_API MCD_ABSTRACT_CLASS IMaterialProperty
{
public:
	virtual ~IMaterialProperty() {}
	virtual IMaterialProperty* clone() const { assert( false ); return nullptr; }

	virtual void begin() const = 0;
	virtual void end() const {}
};

}	// namespace MCD

#include "Color.h"

namespace MCD {

class MCD_RENDER_API ColorProperty : public IMaterialProperty
{
public :
	struct ColorOperation { enum Enum {
		Replace,	//!< Replace the current color
		Add,		//!< Add this color to the current color
		Multiply	//!< Multiply this color with the current color
	}; };

	ColorProperty(const ColorRGBAf& color, ColorOperation::Enum operation);

	//! Apply the color and operation on the destination.
	void apply(ColorRGBAf& dest) const;

protected:
	ColorRGBAf mColor;
	ColorOperation::Enum mOperation;
};	// ColorProperty

/*!	It use static variable to avoid the call of glGetXXX
 */
class MCD_RENDER_API AmbientProperty : public ColorProperty
{
public:
	AmbientProperty(const ColorRGBAf& color, ColorOperation::Enum operation)
		: ColorProperty(color, operation)
	{}

	sal_override IMaterialProperty* clone() const {
		return new AmbientProperty(*this);
	}

	sal_override void begin() const;

private:
	static ColorRGBAf mCurrent;
};	// AmbientProperty

// DiffuseProperty
class MCD_RENDER_API DiffuseProperty : public ColorProperty
{
public:
	DiffuseProperty(const ColorRGBAf& color, ColorOperation::Enum operation)
		: ColorProperty(color, operation)
	{}

	sal_override IMaterialProperty* clone() const {
		return new DiffuseProperty(*this);
	}

	sal_override void begin() const;

private:
	static ColorRGBAf mCurrent;
};	// DiffuseProperty

// SpecularProperty
class MCD_RENDER_API SpecularProperty : public ColorProperty
{
public:
	SpecularProperty(const ColorRGBAf& color, ColorOperation::Enum operation)
		: ColorProperty(color, operation)
	{}

	sal_override IMaterialProperty* clone() const {
		return new SpecularProperty(*this);
	}

	sal_override void begin() const;

private:
	static ColorRGBAf mCurrent;
};	// SpecularProperty

// ShininessProperty
class MCD_RENDER_API ShininessProperty : public IMaterialProperty
{
public:
	explicit ShininessProperty(float value) : mValue(value) {}

	sal_override IMaterialProperty* clone() const {
		return new ShininessProperty(*this);
	}

	sal_override void begin() const;

private:
	float mValue;
};	// ShininessProperty

// StandardProperty
class MCD_RENDER_API StandardProperty : public IMaterialProperty
{
public:
	StandardProperty(
		const ColorRGBAf& ambient,
		const ColorRGBAf& diffuse,
		const ColorRGBAf& specular,
		ColorProperty::ColorOperation::Enum operation,
		float shininess
	);

	sal_override IMaterialProperty* clone() const {
		return new StandardProperty(*this);
	}

	sal_override void begin() const;

private:
	AmbientProperty mAmbient;
	DiffuseProperty mDiffuse;
	SpecularProperty mSpecular;
	ShininessProperty mShininess;
};	// StandardProperty

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

// TextureProperty
class MCD_RENDER_API TextureProperty : public IMaterialProperty
{
public:
	TextureProperty(Texture* texture, int unit);
	TextureProperty(const TextureProperty& rhs);			// Prevent compiler generated copy constructor
	TextureProperty& operator=(const TextureProperty& rhs);	// and assignment operation

	sal_override ~TextureProperty();

	sal_override IMaterialProperty* clone() const;

	sal_override void begin() const;
	sal_override void end() const;

// Attributes
	int unit;
	TexturePtr texture;
	std::string shaderName;	//!< The name of the texture that appear in the shader.
};	// TextureProperty

class ShaderProgram;

// ShaderProperty
// TODO: Introduce ShaderPrarmeterProperty to reduce excessing shader change,
// currently different parameter need to create a different ShaderProgram.
class MCD_RENDER_API ShaderProperty : public IMaterialProperty
{
public:
	explicit ShaderProperty(ShaderProgram* shaderProgram);
	explicit ShaderProperty(const SharedPtr<ShaderProgram>& shaderProgram);
	ShaderProperty(const ShaderProperty& rhs);
	ShaderProperty& operator=(const ShaderProperty& rhs);

	sal_override ~ShaderProperty();

	sal_override IMaterialProperty* clone() const;

	sal_override void begin() const;
	sal_override void end() const;

// Attributes
	SharedPtr<ShaderProgram> shaderProgram;
};	// ShaderProperty

// FrontCullingProperty
class MCD_RENDER_API FrontCullingProperty : public IMaterialProperty
{
public:
	sal_override IMaterialProperty* clone() const {
		return new FrontCullingProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;
};	// FrontCullingProperty

// EnableStateProperty
class MCD_RENDER_API EnableStateProperty : public IMaterialProperty
{
public:
	EnableStateProperty(int state);

	sal_override IMaterialProperty* clone() const {
		return new EnableStateProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;

	int state;
};	// EnableStateProperty

// DisableStateProperty
class MCD_RENDER_API DisableStateProperty : public IMaterialProperty
{
public:
	DisableStateProperty(int state);

	sal_override IMaterialProperty* clone() const {
		return new DisableStateProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;

	int state;
};	// DisableStateProperty

// LineDrawingProperty
class MCD_RENDER_API LineDrawingProperty : public IMaterialProperty
{
public:
	sal_override IMaterialProperty* clone() const {
		return new LineDrawingProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;
};	// LineDrawingProperty

// LineWidthProperty
class MCD_RENDER_API LineWidthProperty : public IMaterialProperty
{
public:
	LineWidthProperty(float width);

	sal_override IMaterialProperty* clone() const {
		return new LineWidthProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;

	float width;
};	// LineWidthProperty

// BlendingProperty
class MCD_RENDER_API BlendingProperty : public IMaterialProperty
{
public:
	BlendingProperty();

	sal_override IMaterialProperty* clone() const {
		return new BlendingProperty(*this);
	}

	sal_override void begin() const;
	sal_override void end() const;

	bool blendEnable;
	bool blendEnableSep;

	int sfactor;
	int dfactor;
	int blendEquation;
	
	int sfactorSep;
	int dfactorSep;
	int blendEquationSep;

};

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIALPROPERTY__
