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

}	// namespace MCD
