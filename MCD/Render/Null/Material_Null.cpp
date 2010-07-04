#include "Pch.h"
#include "../Material.h"

namespace MCD {

MaterialComponent::MaterialComponent()
	: mImpl(*reinterpret_cast<Impl*>(nullptr))
	, diffuseColor(1, 1)
	, specularColor(1, 1)
	, emissionColor(0, 1)
	, specularExponent(20)
	, opacity(1)
{}

MaterialComponent::~MaterialComponent() {}

void MaterialComponent::render2(void* context) {}

void MaterialComponent::preRender(size_t pass, void* context) {}

void MaterialComponent::postRender(size_t pass, void* context) {}

}	// namespace MCD
