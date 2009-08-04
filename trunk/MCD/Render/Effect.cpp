#include "Pch.h"
#include "Effect.h"
#include "Material.h"

namespace MCD {

Effect::Effect(const Path& fileId)
	: Resource(fileId), material(nullptr)
{}

Effect::~Effect()
{
}

}	// namespace MCD
