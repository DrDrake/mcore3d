#ifndef __MCD_RENDER_MATERIAL__
#define __MCD_RENDER_MATERIAL__

#include "Color.h"
#include "MaterialProperty.h"
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

#include "Renderable.h"

namespace MCD {

class RendererComponent;

class MCD_RENDER_API MaterialComponent : public Component
{
public:
	MaterialComponent();

	~MaterialComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(MaterialComponent);
	}

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }

	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render() {}

// Attrubutes
	float specularExponent;

protected:
	friend class RendererComponent;

	//!	Invoked by RendererComponent
	virtual void preRender(size_t pass, void* context);
	virtual void postRender(size_t pass, void* context);

	class Impl;
	Impl& mImpl;
};	// MaterialComponent

typedef IntrusiveWeakPtr<MaterialComponent> MaterialComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
