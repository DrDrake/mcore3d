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
#include "../Component/Render/SkeletonAnimationComponent.h"
#include "../Component/Render/SkinMeshComponent.h"

using namespace MCD;

namespace script {

namespace types {

static void destroy(MCD::AnimationComponent* obj)	{ obj->destroyThis(); }
static void destroy(MCD::CameraComponent* obj)		{ obj->destroyThis(); }
static void destroy(MCD::MeshComponent* obj)		{ obj->destroyThis(); }
static void destroy(MCD::PickComponent* obj)		{ obj->destroyThis(); }

}	// namespace types

// AnimationComponent
static float animationComponentGetTime(AnimationComponent& self) {
	return self.animationInstance.time;
}
static void animationComponentSetTime(AnimationComponent& self, float time) {
	self.animationInstance.time = time;
}
static void animationComponentCallback(AnimationComponent& c, size_t virtualFrameIdx, void* eventData) {
	if(c.scriptCallback.isAssigned())
		c.scriptCallback.call(virtualFrameIdx, &c, reinterpret_cast<const char*>(eventData));
}
static void animationComponentDestroyData(void* data) {
	::free(data);	// The data is the event string passed to animationComponentSetXXXEvent
}
static void animationComponentSetEdgeEvent(AnimationComponent& self, const char* weightedTrackName, size_t virtualFrameIdx, const char* eventString) {
	self.setEdgeEvent(weightedTrackName, virtualFrameIdx, ::strdup(eventString), &animationComponentCallback, &animationComponentDestroyData);
}
static void animationComponentSetLevelEvent(AnimationComponent& self, const char* weightedTrackName, size_t virtualFrameIdx, const char* eventString) {
	self.setLevelEvent(weightedTrackName, virtualFrameIdx, ::strdup(eventString), &animationComponentCallback, &animationComponentDestroyData);
}
SCRIPT_CLASS_REGISTER(AnimationComponent)
	.declareClass<AnimationComponent, Component>("AnimationComponent")
	.enableGetset()
	.constructor<AnimationUpdaterComponent&>()
	.getsetWrapped("time", &animationComponentGetTime, &animationComponentSetTime)
	.scriptEvent("onEvent", &AnimationComponent::scriptCallback)
	.wrappedMethod("setEdgeEvent", &animationComponentSetEdgeEvent)
	.wrappedMethod("setLevelEvent", &animationComponentSetLevelEvent)
//	.runScript("AnimationComponent._cloned<-function(org){_orgCloned(org);onEvent.setHandler(org.getHandler());}")
;}

// AnimationUpdaterComponent
SCRIPT_CLASS_REGISTER(AnimationUpdaterComponent)
	.declareClass<AnimationUpdaterComponent, Component>("AnimationUpdaterComponent")
;}

// CameraComponent
static float cameraComponentGetFov(CameraComponent& self) {
	return self.frustum.fov();
}
static void cameraComponentSetFov(CameraComponent& self, float fov) {
	self.frustum.setFov(fov);
}
SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>("CameraComponent")
	.enableGetset()
	.constructor()
	.getsetWrapped("fov", &cameraComponentGetFov, &cameraComponentSetFov)
;}

// MeshComponent
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
	.declareClass<MeshComponent, Component>("MeshComponent")
	.enableGetset()
	.constructor()
	.getsetWrapped<objRefCount<ResourceRefPolicy> >("mesh", &meshComponentGetMesh, &meshComponentSetMesh)
	.getsetWrapped<objRefCount<ResourceRefPolicy> >("effect", &meshComponentGetEffect, &meshComponentSetEffect)
;}

// PickComponent
static Entity* pickComponentGetEntityToPick(PickComponent& self) {
	return self.entityToPick.get();
}
static void pickComponentSetEntityToPick(PickComponent& self, Entity* e) {
	self.entityToPick = e;
}
static Entity* pickComponentHitAtIndex(PickComponent& self, size_t index) {
	return self.hitAtIndex(index).get();
}
SCRIPT_CLASS_REGISTER_NAME(RenderContext, "RenderContext")
;}

SCRIPT_CLASS_REGISTER(PickComponent)
	.declareClass<PickComponent, Component>("PickComponent")
	.enableGetset()
	.constructor<RenderContext&>()
	.method("_setPickRegion", &PickComponent::setPickRegion)
	.getsetWrapped<objNoCare>("entityToPick", &pickComponentGetEntityToPick, &pickComponentSetEntityToPick)
	.method("_gethitCount", &PickComponent::hitCount)
	.wrappedMethod<objNoCare>("hitAtIndex", &pickComponentHitAtIndex)
	.runScript("PickComponent.setPickRegion<-function(x,y,w=1,h=1){return _setPickRegion(x,y,w,h);}")
	.runScript("PickComponent._gethitResults<-function(){local c=_gethitCount();for(local i=0;i<c;++i)yield hitAtIndex(i);return null;}")	// Generator for foreach
;}

// RenderableComponent
SCRIPT_CLASS_REGISTER(RenderableComponent)
	.declareClass<RenderableComponent, Component>("RenderableComponent")
	.enableGetset()
;}

// SkeletonAnimationComponent
static float skeletonAnimationComponentGetTime(SkeletonAnimationComponent& self) {
	return self.skeletonAnimation.anim.time;
}
static void skeletonAnimationComponentSetTime(SkeletonAnimationComponent& self, float time) {
	self.skeletonAnimation.anim.time = time;
}
SCRIPT_CLASS_REGISTER(SkeletonAnimationComponent)
	.declareClass<SkeletonAnimationComponent, Component>("SkeletonAnimationComponent")
	.enableGetset()
	.getsetWrapped("time", &skeletonAnimationComponentGetTime, &skeletonAnimationComponentSetTime)
;}

// SkeletonAnimationUpdaterComponent
SCRIPT_CLASS_REGISTER(SkeletonAnimationUpdaterComponent)
	.declareClass<SkeletonAnimationUpdaterComponent, Component>("SkeletonAnimationUpdaterComponent")
;}

// SkinMeshComponent
static SkeletonAnimationComponent* skinMeshComponentGetSkeletonAnimation(SkinMeshComponent& self) {
	return self.skeletonAnimation.get();
}
static void skinMeshComponentSetSkeletonAnimation(SkinMeshComponent& self, SkeletonAnimationComponent* c) {
	self.skeletonAnimation = c;
}
SCRIPT_CLASS_REGISTER(SkinMeshComponent)
	.declareClass<SkinMeshComponent, Component>("SkinMeshComponent")
	.enableGetset()
	.getsetWrapped<objNoCare>("skeletonAnimation", &skinMeshComponentGetSkeletonAnimation, &skinMeshComponentSetSkeletonAnimation)
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
	script::ClassTraits<SkeletonAnimationComponent>::bind(v);
	script::ClassTraits<SkinMeshComponent>::bind(v);
}

}	// namespace MCD
