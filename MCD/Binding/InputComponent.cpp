#include "Pch.h"
#include "InputComponent.h"
#include "Entity.h"
#include "Math.h"

using namespace MCD;

namespace script {

static Vec2f inputComponentGetMousePosition(InputComponent& self) {
	// Transform the result from type Vec2i to Vec2f
	Vec2i pos = self.getMousePosition();
	return Vec2f(pos.x, pos.y);
}
SCRIPT_CLASS_REGISTER(InputComponent)
	.declareClass<InputComponent, Component>(L"InputComponent")
	.enableGetset(L"InputComponent")
	.method(L"anyKey", &InputComponent::anyKey)
	.method(L"anyKeyDown", &InputComponent::anyKeyDown)
	.method(L"getAxis", &InputComponent::getAxis)
	.method(L"getButton", &InputComponent::getButton)
	.method(L"getButtonDown", &InputComponent::getButtonDown)
	.method(L"getButtonUp", &InputComponent::getButtonUp)
	.wrappedMethod(L"_getmousePosition", &inputComponentGetMousePosition)
	.method(L"getMouseButton", &InputComponent::getMouseButton)
	.method(L"getMouseButtonDown", &InputComponent::getMouseButtonDown)
	.method(L"getMouseButtonUp", &InputComponent::getMouseButtonUp)
	.method(L"_getinputString", &InputComponent::inputString)
;}

}	// namespace script

namespace MCD {

void registerInputComponentBinding(script::VMCore* v)
{
	script::ClassTraits<InputComponent>::bind(v);
}

}	// namespace MCD
