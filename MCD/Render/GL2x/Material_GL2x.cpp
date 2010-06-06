#include "Pch.h"
#include "../Material.h"

namespace MCD {

MaterialComponent::MaterialComponent()
	: mImpl(*reinterpret_cast<Impl*>(nullptr))
	, specularExponent(20)
{
}

MaterialComponent::~MaterialComponent()
{
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	float color[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	glMaterialfv(GL_FRONT, GL_SPECULAR, color);
	glMaterialf(GL_FRONT, GL_SHININESS, specularExponent);

	if(diffuseMap)
		diffuseMap->bind();
}

void MaterialComponent::postRender(size_t pass, void* context)
{
	if(diffuseMap)
		diffuseMap->unbind();
}

}	// namespace MCD
