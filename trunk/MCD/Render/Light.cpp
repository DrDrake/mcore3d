#include "Pch.h"
#include "Light.h"

namespace MCD {

LightComponent::~LightComponent() {}

Component* LightComponent::clone() const
{
	LightComponent* ret = new LightComponent;
	ret->type = type;
	ret->color = color;
	return ret;
}

}	// namespace MCD
