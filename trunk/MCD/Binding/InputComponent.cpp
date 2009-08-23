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
	.declareClass<InputComponent, Component>(xSTRING("InputComponent"))
	.enableGetset()
	.method(xSTRING("anyKey"), &InputComponent::anyKey)
	.method(xSTRING("anyKeyDown"), &InputComponent::anyKeyDown)
	.method(xSTRING("getAxis"), &InputComponent::getAxis)
	.method(xSTRING("getAxisRaw"), &InputComponent::getAxisRaw)
	.method(xSTRING("getButton"), &InputComponent::getButton)
	.method(xSTRING("getButtonDown"), &InputComponent::getButtonDown)
	.method(xSTRING("getButtonUp"), &InputComponent::getButtonUp)
	.wrappedMethod(xSTRING("_getmousePosition"), &inputComponentGetMousePosition)
	.method(xSTRING("getMouseButton"), &InputComponent::getMouseButton)
	.method(xSTRING("getMouseButtonDown"), &InputComponent::getMouseButtonDown)
	.method(xSTRING("getMouseButtonUp"), &InputComponent::getMouseButtonUp)
	.method(xSTRING("_getinputString"), &InputComponent::inputString)
;}

}	// namespace script

namespace MCD {

void registerInputComponentBinding(script::VMCore* v)
{
	script::ClassTraits<InputComponent>::bind(v);
}

}	// namespace MCD
