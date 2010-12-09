#ifndef __MCD_RENDER_MATERIAL__
#define __MCD_RENDER_MATERIAL__

#include "Color.h"
#include "Renderable.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

class RendererComponent;
typedef IntrusivePtr<class Texture> TexturePtr;

class MCD_ABSTRACT_CLASS IMaterialComponent : public RenderableComponent
{
public:
	virtual ~IMaterialComponent() {}
	//!	Invoked by RendererComponent
	virtual void preRender(size_t pass, void* context) = 0;
	virtual void postRender(size_t pass, void* context) = 0;

	virtual bool isTransparent() const = 0;
};	// IMaterialComponent

class MCD_RENDER_API MaterialComponent : public IMaterialComponent
{
public:
	MaterialComponent();

// Cloning
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render(sal_in void* context);

// Attrubutes
	ColorRGBAf diffuseColor;
	ColorRGBAf specularColor;
	ColorRGBAf emissionColor;
	float specularExponent;
	float opacity;	//!< Value from 0 (invisible) to 1 (opaque)

	bool lighting;
	bool cullFace;
	bool useVertexColor;

	TexturePtr diffuseMap;
	TexturePtr specularMap;
	TexturePtr bumpMap;
	float bumpFactor;

	sal_override bool isTransparent() const { return opacity < 1; }

protected:
	sal_override ~MaterialComponent();

	friend class RendererComponent;

	//!	Invoked by RendererComponent
	sal_override void preRender(size_t pass, void* context);
	sal_override void postRender(size_t pass, void* context);

	class Impl;
	Impl& mImpl;
};	// MaterialComponent

typedef IntrusiveWeakPtr<MaterialComponent> MaterialComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
