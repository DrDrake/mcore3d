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
}

void MaterialComponent::postRender(size_t pass, void* context)
{
}

}	// namespace MCD
