#include "Pch.h"
#include "../Material.h"
#include "Renderer.inc"

namespace MCD {

MaterialComponent::MaterialComponent()
	: mImpl(*reinterpret_cast<Impl*>(nullptr))
	, diffuseColor(1, 1)
	, specularColor(1, 1)
	, emissionColor(0, 1)
	, specularExponent(20)
	, opacity(1)
{
}

MaterialComponent::~MaterialComponent()
{
}

void MaterialComponent::render2(void* context)
{
	// Push light into Renderer's light list
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void MaterialComponent::preRender(size_t pass, void* context)
{
	ColorRGBAf diffuse(diffuseColor.rgb(), opacity);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.rawPointer());

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor.rawPointer());
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionColor.rawPointer());
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, specularExponent);

	if(diffuseMap)
		diffuseMap->bind();
}

void MaterialComponent::postRender(size_t pass, void* context)
{
	if(diffuseMap)
		diffuseMap->unbind();
}

void SpriteMaterialComponent::render2(void* context)
{
	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.mCurrentMaterial = this;
}

void SpriteMaterialComponent::preRender(size_t pass, void* context)
{
	if(diffuseMap)
		diffuseMap->bind();
}

void SpriteMaterialComponent::postRender(size_t pass, void* context)
{
	if(diffuseMap)
		diffuseMap->unbind();
}

}	// namespace MCD
