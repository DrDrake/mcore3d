#include "Pch.h"
#include "InputComponent.h"
#include "Entity.h"

using namespace MCD;

namespace script {

namespace types {

void addHandleToObject(HSQUIRRELVM v, WinMessageInputComponent* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, WinMessageInputComponent* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

}	// namespace types

SCRIPT_CLASS_REGISTER(WinMessageInputComponent)
	.declareClass<WinMessageInputComponent, Component>(L"InputComponent")
	.enableGetset(L"InputComponent")
	.constructor()
	.method(L"anyKey", &WinMessageInputComponent::anyKey)
	.method(L"anyKeyDown", &WinMessageInputComponent::anyKeyDown)
	.method(L"getAxis", &WinMessageInputComponent::getAxis)
	.method(L"getButton", &WinMessageInputComponent::getButton)
	.method(L"getButtonDown", &WinMessageInputComponent::getButtonDown)
	.method(L"getButtonUp", &WinMessageInputComponent::getButtonUp)
//	.getset(L"velocity", &InputComponent::velocity)
;}

}	// namespace script

namespace MCD {

void registerInputComponentBinding(script::VMCore* v)
{
	script::ClassTraits<WinMessageInputComponent>::bind(v);
}

}	// namespace MCD
