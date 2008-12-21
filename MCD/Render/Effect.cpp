#include "Pch.h"
#include "Effect.h"
#include "Material.h"
#include "Texture.h"

namespace MCD {

Effect::Effect(const Path& fileId)
	: Resource(fileId), material(nullptr)
{}

Effect::~Effect()
{
	delete material;
}

}	// namespace MCD
