#include "Pch.h"
#include "InputComponent.h"
#include "Entity.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER(InputComponent)
	.declareClass<InputComponent, Component>(L"InputComponent")
	.method(L"anyKey", &InputComponent::anyKey)
	.method(L"anyKeyDown", &InputComponent::anyKeyDown)
	.method(L"getAxis", &InputComponent::getAxis)
	.method(L"getButton", &InputComponent::getButton)
	.method(L"getButtonDown", &InputComponent::getButtonDown)
	.method(L"getButtonUp", &InputComponent::getButtonUp)
//	.getset(L"velocity", &InputComponent::velocity)
;}

}	// namespace script

namespace MCD {

void registerInputComponentBinding(script::VMCore* v)
{
	script::ClassTraits<InputComponent>::bind(v);
}

}	// namespace MCD
