#include "Pch.h"
#include "Prefab.h"

namespace MCD {

Prefab::Prefab(const Path& fileId)
	: Resource(fileId), reloadCount(0)
{
}

Prefab::~Prefab()
{
}

}	// namespace MCD
