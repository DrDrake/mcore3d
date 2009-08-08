#include "Pch.h"
#include "RenderComponent.h"
#include "Binding.h"
#include "Math.h"
#include "Render.h"
#include "System.h"
#include "../Render/Effect.h"
#include "../Render/Mesh.h"
#include "../Component/Render/CameraComponent.h"
#include "../Component/Render/MeshComponent.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>(L"CameraComponent")
	.enableGetset(L"CameraComponent")
	.constructor()
	.getset(L"velocity", &CameraComponent::velocity)
;}

static Mesh* meshComponentGetMesh(MeshComponent& self) {
	return self.mesh.get();
}
static void meshComponentSetMesh(MeshComponent& self, Mesh* mesh) {
	self.mesh = mesh;
}
static Effect* meshComponentGetEffect(MeshComponent& self) {
	return self.effect.get();
}
static void meshComponentSetEffect(MeshComponent& self, Effect* effect) {
	self.effect = effect;
}

SCRIPT_CLASS_REGISTER(MeshComponent)
	.declareClass<MeshComponent, Component>(L"MeshComponent")
	.enableGetset(L"MeshComponent")
	.constructor()
	.wrappedMethod<objRefCount<ResourceRefPolicy> >(L"_getmesh", &meshComponentGetMesh)
	.wrappedMethod(L"_setmesh", &meshComponentSetMesh)
	.wrappedMethod<objRefCount<ResourceRefPolicy> >(L"_geteffect", &meshComponentGetEffect)
	.wrappedMethod(L"_seteffect", &meshComponentSetEffect)
;}

SCRIPT_CLASS_REGISTER(RenderableComponent)
	.declareClass<RenderableComponent, Component>(L"RenderableComponent")
	.enableGetset(L"RenderableComponent")
;}

}	// namespace script

namespace MCD {

void registerRenderComponentBinding(script::VMCore* v)
{
	script::ClassTraits<CameraComponent>::bind(v);
	script::ClassTraits<MeshComponent>::bind(v);
	script::ClassTraits<RenderableComponent>::bind(v);
}

}	// namespace MCD
