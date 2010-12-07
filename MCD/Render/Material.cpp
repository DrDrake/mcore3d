#include "Pch.h"
#include "Material.h"
#include "Texture.h"

namespace MCD {

Component* MaterialComponent::clone() const
{
	MaterialComponent* cloned = new MaterialComponent;
	cloned->diffuseColor = diffuseColor;
	cloned->specularColor = specularColor;
	cloned->emissionColor = emissionColor;
	cloned->specularExponent = specularExponent;
	cloned->opacity = opacity;
	cloned->lighting = lighting;
	cloned->cullFace = cullFace;
	cloned->useVertexColor = useVertexColor;
	cloned->diffuseMap = diffuseMap;
	cloned->bumpMap = bumpMap;
	cloned->bumpFactor = bumpFactor;
	return cloned;
}

SpriteMaterialComponent::SpriteMaterialComponent()
	: opacity(1)
{}

SpriteMaterialComponent::~SpriteMaterialComponent()
{}

Component* SpriteMaterialComponent::clone() const
{
	SpriteMaterialComponent* cloned = new SpriteMaterialComponent;
	cloned->opacity = this->opacity;
	return cloned;
}

}	// namespace MCD
