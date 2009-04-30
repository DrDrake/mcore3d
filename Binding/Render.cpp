#include "Pch.h"
#include "Render.h"
#include "Binding.h"
#include "Entity.h"
#include "Math.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER(CameraComponentPtr)
	.declareClass<CameraComponentPtr, ComponentPtr>(L"CameraComponentPtr")
	.constructor<CameraComponent*>(L"create")
	.method<objNoCare>(L"getPointee", &CameraComponentPtr::get)
;}

SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>(L"CameraComponent")
	.enableGetset(L"CameraComponent")
	.constructor(L"create")
	.getset(L"velocity", &CameraComponent::velocity)
;}

}	// namespace script

namespace MCD {

void registerRenderBinding(script::VMCore* v)
{
	script::ClassTraits<CameraComponent>::bind(v);
	script::ClassTraits<CameraComponentPtr>::bind(v);
}

}	// namespace MCD
