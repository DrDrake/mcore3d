#include "Pch.h"
#include "Effect.h"
#include "Material.h"
#include "Texture.h"	// TODO: Remove this dependency

namespace MCD {

Effect::Effect(const Path& fileId)
	: Resource(fileId), material(nullptr)
{}

Effect::~Effect()
{
}

}	// namespace MCD
