#include "Pch.h"
#include "Material.h"
#include "Texture.h"
#include "../../3Party/glew/glew.h"
#include <algorithm>
#include <functional>

using namespace std;

namespace MCD {

Material::Material()
	: ambient(0.5), diffuse(1), specular(0), shininess(0)
{
}

// The destructor is implemented in cpp file so that Material.h need not to
// include Texture.h because we have a TexturePtr as member variable
Material::~Material()
{
}

void Material::bind() const
{
	{	GLfloat a[] = { ambient.r, ambient.g, ambient.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
	}

	{	GLfloat d[] = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
	}

	{	GLfloat s[] = { specular.r, specular.g, specular.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	}

	if(texture) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->bind();
	}
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

Material2::Pass::~Pass()
{
}

void Material2::Pass::addProperty(IMaterialProperty* property)
{
	mProperty.push_back(property);
}

void Material2::Pass::preRender() const
{
	for_each(mProperty.begin(), mProperty.end(), mem_fun_ref(&IMaterialProperty::begin));
}

void Material2::Pass::postRender() const
{
	for_each(mProperty.begin(), mProperty.end(), mem_fun_ref(&IMaterialProperty::end));
}

Material2::~Material2()
{
}

IMaterial* Material2::clone() const
{
	std::auto_ptr<Material2> newMaterial(new Material2);

	// Clone all material properties in all pass to the new material
	for(size_t i=0; i<mRenderPasses.size(); ++i) {
		for(Pass::PropertyList::const_iterator j=mRenderPasses[i].mProperty.begin(); j!=mRenderPasses[i].mProperty.end(); ++j)
			newMaterial->addProperty(j->clone(), i);
	}

	return newMaterial.release();
}

void Material2::preRender(size_t pass)
{
	MCD_ASSERT(pass < mRenderPasses.size());
	mRenderPasses[pass].preRender();
}

void Material2::postRender(size_t pass)
{
	MCD_ASSERT(pass < mRenderPasses.size());
	mRenderPasses[pass].postRender();
}

size_t Material2::getPassCount() const
{
	return mRenderPasses.size();
}

void Material2::addProperty(IMaterialProperty* property, size_t pass)
{
	// Create the necessery pass first
	while(pass >= mRenderPasses.size())
		mRenderPasses.push_back(new Pass);

	static_cast<Pass&>(mRenderPasses[pass]).addProperty(property);
}

}	// namespace MCD
