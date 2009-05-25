#include "Pch.h"
#include "Render.h"
#include "Binding.h"
#include "Entity.h"
#include "Math.h"

using namespace MCD;

namespace script {

namespace types {

void addHandleToObject(HSQUIRRELVM v, CameraComponent* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, CameraComponent* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

}	// namespace types

SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>(L"CameraComponent")
	.enableGetset(L"CameraComponent")
	.constructor()
	.getset(L"velocity", &CameraComponent::velocity)
;}

}	// namespace script

namespace MCD {

void registerRenderBinding(script::VMCore* v)
{
	script::ClassTraits<CameraComponent>::bind(v);
}

}	// namespace MCD
