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

SCRIPT_CLASS_REGISTER(MeshComponent)
	.declareClass<MeshComponent, Component>(L"MeshComponent")
	.enableGetset(L"MeshComponent")
;}

SCRIPT_CLASS_REGISTER(RenderableComponent)
	.declareClass<RenderableComponent, Component>(L"RenderableComponent")
	.enableGetset(L"RenderableComponent")
;}

}	// namespace script

namespace MCD {

void registerRenderBinding(script::VMCore* v)
{
	script::ClassTraits<CameraComponent>::bind(v);
	script::ClassTraits<MeshComponent>::bind(v);
	script::ClassTraits<RenderableComponent>::bind(v);
}

}	// namespace MCD
