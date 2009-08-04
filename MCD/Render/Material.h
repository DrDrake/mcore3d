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
	virtual IMaterial* clone() const = 0;
	virtual void preRender(size_t pass) = 0;
	virtual void postRender(size_t pass) = 0;
	virtual size_t getPassCount() const = 0;
	virtual void addProperty(IMaterialProperty* property, size_t pass) = 0;
};

class MCD_RENDER_API Material : public IMaterial
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
	};

	Material() {}
	sal_override ~Material();

// Operations
	sal_override Material* clone() const;

	sal_override void preRender(size_t pass);

	sal_override void postRender(size_t pass);

	sal_override size_t getPassCount() const;

	sal_override void addProperty(sal_in IMaterialProperty* property, size_t pass);

//protected:
	typedef ptr_vector<Pass> RenderPasses;
	RenderPasses mRenderPasses;
};	// Material

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
