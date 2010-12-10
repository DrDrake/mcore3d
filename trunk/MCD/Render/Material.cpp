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
	cloned->alphaMap = alphaMap;
	cloned->diffuseMap = diffuseMap;
	cloned->emissionMap = emissionMap;
	cloned->specularMap = specularMap;
	cloned->bumpMap = bumpMap;
	cloned->bumpFactor = bumpFactor;
	return cloned;
}

bool MaterialComponent::isTransparent() const {
	return opacity < 1 || alphaMap != nullptr;
}

}	// namespace MCD
