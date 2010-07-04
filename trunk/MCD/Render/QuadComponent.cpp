#include "Pch.h"
#include "QuadComponent.h"

namespace MCD {

QuadComponent::QuadComponent()
	: width(0), height(0)
{
	uv = Vec4f(0, 0, 1, 1);
}

QuadComponent::~QuadComponent() {}

bool QuadComponent::cloneable() const { return true; }

Component* QuadComponent::clone() const
{
	QuadComponent* ret = new QuadComponent;
	ret->width = width;
	ret->height = height;
	ret->uv = uv;
	return ret;
}

void QuadComponent::render() {}

}	// namespace MCD
