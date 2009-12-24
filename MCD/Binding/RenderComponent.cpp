#include "Pch.h"
#include "RenderComponent.h"
#include "Binding.h"
#include "Entity.h"
#include "Math.h"
#include "Render.h"
#include "System.h"
#include "../Render/Effect.h"
#include "../Render/Mesh.h"
#include "../Component/Render/AnimationComponent.h"
#include "../Component/Render/CameraComponent.h"
#include "../Component/Render/MeshComponent.h"
#include "../Component/Render/PickComponent.h"

using namespace MCD;

namespace script {

// TODO: Remove this temporary solution
static AnimationThread thread;
static AnimationComponent* animationComponentCreate() {
	return new AnimationComponent(thread);
}
SCRIPT_CLASS_REGISTER(AnimationComponent)
	.declareClass<AnimationComponent, Component>(xSTRING("AnimationComponent"))
	.staticMethod<construct>(xSTRING("constructor"), &animationComponentCreate)
;}

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

static Entity* pickComponentGetEntityToPick(PickComponent& self) {
	return self.entityToPick.get();
}
static void pickComponentSetEntityToPick(PickComponent& self, Entity* e) {
	self.entityToPick = e;
}
static Entity* pickComponentHitAtIndex(PickComponent& self, size_t index) {
	return self.hitAtIndex(index).get();
}
SCRIPT_CLASS_REGISTER(PickComponent)
	.declareClass<PickComponent, Component>(xSTRING("PickComponent"))
	.enableGetset()
	.constructor()
	.method(xSTRING("_setPickRegion"), &PickComponent::setPickRegion)
	.wrappedMethod<objNoCare>(xSTRING("_getentityToPick"), &pickComponentGetEntityToPick)
	.wrappedMethod(xSTRING("_setentityToPick"), &pickComponentSetEntityToPick)
	.method(xSTRING("_gethitCount"), &PickComponent::hitCount)
	.wrappedMethod<objNoCare>(xSTRING("hitAtIndex"), &pickComponentHitAtIndex)
	.runScript(xSTRING("PickComponent.setPickRegion<-function(x,y,w=1,h=1){return _setPickRegion(x,y,w,h);}"))
	.runScript(xSTRING("PickComponent._gethitResults<-function(){local c=_gethitCount();for(local i=0;i<c;++i)yield hitAtIndex(i);return null;}"))	// Generator for foreach
;}

SCRIPT_CLASS_REGISTER(RenderableComponent)
	.declareClass<RenderableComponent, Component>(xSTRING("RenderableComponent"))
	.enableGetset()
;}

}	// namespace script

namespace MCD {

void registerRenderComponentBinding(script::VMCore* v)
{
	script::ClassTraits<AnimationComponent>::bind(v);
	script::ClassTraits<CameraComponent>::bind(v);
	script::ClassTraits<MeshComponent>::bind(v);
	script::ClassTraits<PickComponent>::bind(v);
	script::ClassTraits<RenderableComponent>::bind(v);
}

}	// namespace MCD
