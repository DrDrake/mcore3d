#include "Pch.h"
#include "../Material.h"
#include "../Texture.h"
#include "Renderer.inc"

namespace MCD {

MaterialComponent::MaterialComponent()
	: mImpl(*reinterpret_cast<Impl*>(nullptr))
	, diffuseColor(1, 1)
	, specularColor(1, 1)
	, emissionColor(0, 1)
	, specularExponent(20)
	, opacity(1)
	, lighting(true)
	, cullFace(true)
	, useVertexColor(false)
	, bumpFactor(1)
{
}

MaterialComponent::~MaterialComponent()
{
}

void MaterialComponent::render(void* context)
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

	if(!lighting)
		glDisable(GL_LIGHTING);

	if(cullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if(useVertexColor)
		glEnable(GL_COLOR_MATERIAL);
	else
		glDisable(GL_COLOR_MATERIAL);
}

void MaterialComponent::postRender(size_t pass, void* context)
{
	if(diffuseMap)
		diffuseMap->unbind();

	// Restore the lighting
	if(!lighting) {
		RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
		if(renderer.mLights.empty())
			glDisable(GL_LIGHTING);
		else
			glEnable(GL_LIGHTING);
	}
}

}	// namespace MCD
