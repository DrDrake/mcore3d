#include "Pch.h"
#include "QuadComponent.h"

namespace MCD {

QuadComponent::QuadComponent()
	: width(0), height(0)
{}

QuadComponent::~QuadComponent() {}

bool QuadComponent::cloneable() const { return true; }

Component* QuadComponent::clone() const
{
	QuadComponent* ret = new QuadComponent;
	ret->width = width;
	ret->height = height;
	return ret;
}

void QuadComponent::render() {}

}	// namespace MCD
