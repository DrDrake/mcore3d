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

static float cameraComponentGetFov(CameraComponent& self) {
	return self.camera.frustum.fov();
}
static void cameraComponentSetFov(CameraComponent& self, float fov) {
	self.camera.frustum.setFov(fov);
}
SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>(xSTRING("CameraComponent"))
	.enableGetset()
	.constructor()
	.wrappedMethod(xSTRING("_getfov"), &cameraComponentGetFov)
	.wrappedMethod(xSTRING("_setfov"), &cameraComponentSetFov)
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
	.declareClass<MeshComponent, Component>(xSTRING("MeshComponent"))
	.enableGetset()
	.constructor()
	.wrappedMethod<objRefCount<ResourceRefPolicy> >(xSTRING("_getmesh"), &meshComponentGetMesh)
	.wrappedMethod(xSTRING("_setmesh"), &meshComponentSetMesh)
	.wrappedMethod<objRefCount<ResourceRefPolicy> >(xSTRING("_geteffect"), &meshComponentGetEffect)
	.wrappedMethod(xSTRING("_seteffect"), &meshComponentSetEffect)
;}

SCRIPT_CLASS_REGISTER(RenderableComponent)
	.declareClass<RenderableComponent, Component>(xSTRING("RenderableComponent"))
	.enableGetset()
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
