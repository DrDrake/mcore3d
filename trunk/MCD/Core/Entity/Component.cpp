#include "Pch.h"
#include "Component.h"
#include "../System/Utility.h"

namespace MCD {

Component::Component()
	: mEntity(nullptr)
{
}

Entity* Component::entity() {
	return mEntity;
}

}	// namespace MCD
