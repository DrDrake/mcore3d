#ifndef __MCD_RENDER_MATERIAL__
#define __MCD_RENDER_MATERIAL__

#include "Color.h"
#include "MaterialProperty.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/PtrVector.h"

namespace MCD {

typedef IntrusivePtr<class Texture> TexturePtr;

class MCD_RENDER_API Material
{
public:
	Material();
	~Material();

// Operations
	void bind() const;

// Attributes
	ColorRGBf ambient, diffuse, specular;
	uint8_t shininess;
	TexturePtr texture;
};	// Material

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

class MCD_RENDER_API Material2 : public IMaterial
{
public:
	// TODO: We may separate mProperty into several list as an optimization
	// to reduce virtual function call overheads.
	class Pass : public IRenderPass
	{
	public:
		sal_override ~Pass();

		void addProperty(IMaterialProperty* property);

		sal_override void preRender() const;
		sal_override void postRender() const;

		typedef ptr_vector<IMaterialProperty> PropertyList;
		PropertyList mProperty;
	};

	Material2() {}
	sal_override ~Material2();

// Operations
	sal_override IMaterial* clone() const;

	sal_override void preRender(size_t pass);

	sal_override void postRender(size_t pass);

	sal_override size_t getPassCount() const;

	sal_override void addProperty(IMaterialProperty* property, size_t pass);

//protected:
	typedef ptr_vector<Pass> RenderPasses;
	RenderPasses mRenderPasses;
};	// Material2

}	// namespace MCD

#endif	// __MCD_RENDER_MATERIAL__
