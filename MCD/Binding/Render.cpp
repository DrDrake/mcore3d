#include "Pch.h"
#include "Render.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

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