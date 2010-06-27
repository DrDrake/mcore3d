#include "Pch.h"
#include "Light.h"

namespace MCD {

LightComponent::~LightComponent() {}

bool LightComponent::cloneable() const { return true; }

Component* LightComponent::clone() const
{
	LightComponent* ret = new LightComponent;
	ret->type = type;
	ret->color = color;
	return ret;
}

void LightComponent::render() {}

}	// namespace MCD
