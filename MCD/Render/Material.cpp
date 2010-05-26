#include "Pch.h"
#include "Material.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"
#include <algorithm>
#include <functional>
#include <memory>	// For auto_ptr

using namespace std;

namespace MCD {

Material::Pass::~Pass()
{
}

void Material::Pass::addProperty(IMaterialProperty* property)
{
	MCD_ASSERT(property != nullptr);
	mProperty.push_back(property);
}

void Material::Pass::preRender() const
{
	for_each(mProperty.begin(), mProperty.end(), mem_fun_ref(&IMaterialProperty::begin));
}

void Material::Pass::postRender() const
{
	for_each(mProperty.begin(), mProperty.end(), mem_fun_ref(&IMaterialProperty::end));
}

TextureProperty* Material::Pass::textureProp(int unit)
{
	PropertyList::iterator iProp = mProperty.begin();
	while(iProp != mProperty.end())
	{
		TextureProperty* texProp = dynamic_cast<TextureProperty*>(&(*iProp));

		if(texProp && texProp->unit == unit)
			return texProp;

		++iProp;
	}

	return nullptr;
}

ShaderProperty* Material::Pass::shaderProp()
{
	PropertyList::iterator iProp = mProperty.begin();
	while(iProp != mProperty.end())
	{
		ShaderProperty* shdProp = dynamic_cast<ShaderProperty*>(&(*iProp));

		if(shdProp)
			return shdProp;

		++iProp;
	}

	return nullptr;
}

Material::~Material()
{
}

Material* Material::clone() const
{
	std::auto_ptr<Material> newMaterial(new Material);

	// Clone all material properties in all pass to the new material
	for(size_t i=0; i<mRenderPasses.size(); ++i) {
		for(Pass::PropertyList::const_iterator j=mRenderPasses[i].mProperty.begin(); j!=mRenderPasses[i].mProperty.end(); ++j)
			newMaterial->addProperty(j->clone(), i);
	}

	return newMaterial.release();
}

void Material::preRender(size_t pass)
{
	MCD_ASSERT(pass < mRenderPasses.size());
	mRenderPasses[pass].preRender();
}

void Material::postRender(size_t pass)
{
	MCD_ASSERT(pass < mRenderPasses.size());
	mRenderPasses[pass].postRender();
}

size_t Material::getPassCount() const
{
	return mRenderPasses.size();
}

void Material::addProperty(IMaterialProperty* property, size_t pass)
{
	// Create the necessery pass first
	while(pass >= mRenderPasses.size())
		mRenderPasses.push_back(new Pass);

	static_cast<Pass&>(mRenderPasses[pass]).addProperty(property);
}

Component* MaterialComponent::clone() const
{
	MaterialComponent* cloned = new MaterialComponent;
	cloned->specularExponent = this->specularExponent;
	return cloned;
}

}	// namespace MCD
