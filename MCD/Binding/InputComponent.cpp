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
	.declareClass<InputComponent, Component>("InputComponent")
	.enableGetset()
	.method("anyKey", &InputComponent::anyKey)
	.method("anyKeyDown", &InputComponent::anyKeyDown)
	.method("getAxis", &InputComponent::getAxis)
	.method("getAxisRaw", &InputComponent::getAxisRaw)
	.method("getAxisDelta", &InputComponent::getAxisDelta)
	.method("getAxisDeltaRaw", &InputComponent::getAxisDeltaRaw)
	.method("getButton", &InputComponent::getButton)
	.method("getButtonDown", &InputComponent::getButtonDown)
	.method("getButtonUp", &InputComponent::getButtonUp)
	.wrappedMethod("_getmousePosition", &inputComponentGetMousePosition)
	.method("getMouseButton", &InputComponent::getMouseButton)
	.method("getMouseButtonDown", &InputComponent::getMouseButtonDown)
	.method("getMouseButtonUp", &InputComponent::getMouseButtonUp)
	.method("_getinputString", &InputComponent::inputString)
;}

}	// namespace script

namespace MCD {

void registerInputComponentBinding(script::VMCore* v)
{
	script::ClassTraits<InputComponent>::bind(v);
}

}	// namespace MCD
