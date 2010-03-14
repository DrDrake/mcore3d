#include "Pch.h"
#include "Component.h"
#include "Binding.h"
#include "../Component/Render/EntityPrototype.h"
#include "../Component/PrefabLoaderComponent.h"

using namespace MCD;

namespace script {

namespace types {

static void destroy(MCD::PrefabLoaderComponent* obj)	{ obj->destroyThis(); }

}	// namespace types

SCRIPT_CLASS_REGISTER(PrefabLoaderComponent)
	.declareClass<PrefabLoaderComponent, Component>(xSTRING("PrefabLoaderComponent"))
	.constructor()
;}

}	// namespace script

namespace MCD {

void registerComponentBinding(script::VMCore* v)
{
	script::ClassTraits<PrefabLoaderComponent>::bind(v);
}

}	// namespace MCD
