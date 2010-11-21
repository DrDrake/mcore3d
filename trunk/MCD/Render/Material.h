#ifndef __MCD_RENDER_MATERIAL__
#define __MCD_RENDER_MATERIAL__

#include "Color.h"
#include "MaterialProperty.h"
#include "Renderable.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/PtrVector.h"

namespace MCD {

typedef IntrusivePtr<class Texture> TexturePtr;

class MCD_ABSTRACT_CLASS IRenderPass
{
public :
	virtual ~IRenderPass() {}
	virtual void preRender() const = 0;
	virtual void postRender() const = 0;
};	// IRenderPass

class MCD_ABSTRACT_CLASS IMaterial
{
public:
	virtual ~IMaterial() {}
	virtual sal_notnull IMaterial* clone() const = 0;
	virtual void preRender(size_t pass) = 0;
	virtual void postRender(size_t pass) = 0;
	virtual size_t getPassCount() const = 0;
	virtual void addProperty(IMaterialProperty* property, size_t pass) = 0;
};	// IMaterial

class MCD_RENDER_API Material : public IMaterial, Noncopyable
{
public:
	// TODO: We may separate mProperty into several list as an optimization
	// to reduce virtual function call overheads.
	class MCD_RENDER_API Pass : public IRenderPass
	{
	public:
		sal_override ~Pass();

		void addProperty(sal_in IMaterialProperty* property);

		sal_override void preRender() const;
		sal_override void postRender() const;

		typedef ptr_vector<IMaterialProperty> PropertyList;
		PropertyList mProperty;

		/*!	Returns the TextureProperty at a specific unit;
			nullptr is return if no such unit.
		 */
		sal_maybenull TextureProperty* textureProp(int unit);

		//!	Returns the ShaderProperty; nullptr is return if no such property.
		sal_maybenull ShaderProperty* shaderProp();
	};	// Pass

	Material() {}
	sal_override ~Material();

// Operations
	sal_override sal_notnull Material* clone() const;

	sal_override void preRender(size_t pass);

	sal_override void postRender(size_t pass);

	sal_override size_t getPassCount() const;

	sal_override void addProperty(sal_in IMaterialProperty* property, size_t pass);

//protected:
	typedef ptr_vector<Pass> RenderPasses;
	RenderPasses mRenderPasses;
};	// Material

}	// namespace MCD

namespace MCD {

class RendererComponent;

class MCD_ABSTRACT_CLASS IMaterialComponent : public RenderableComponent
{
public:
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
	TexturePtr bumpMap;

	sal_override bool isTransparent() const { return opacity < 1; }

protected:
	~MaterialComponent();

	friend class RendererComponent;

	//!	Invoked by RendererComponent
	sal_override void preRender(size_t pass, void* context);
	sal_override void postRender(size_t pass, void* context);

	class Impl;
	Impl& mImpl;
};	// MaterialComponent

typedef IntrusiveWeakPtr<MaterialComponent> MaterialComponentPtr;

/*!	A material that do not react with lighting, simply defined by a texture and an opacity value.
 */
class MCD_RENDER_API SpriteMaterialComponent : public IMaterialComponent
{
public:
	SpriteMaterialComponent();

// Cloning
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render(sal_in void* context);

// Attrubutes
	float opacity;	//!< Value from 0 (invisible) to 1 (opaque)
	TexturePtr diffuseMap;

	sal_override bool isTransparent() const { return opacity < 1; }

protected:
	~SpriteMaterialComponent() {}

	//!	Invoked by RendererComponent
	sal_override void preRender(size_t pass, void* context);
	sal_override void postRender(size_t pass, void* context);
};	// SpriteMaterialComponent

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
