#include "Pch.h"
#include "Render.h"
#include "Binding.h"
#include "Entity.h"
#include "Math.h"

using namespace MCD;

namespace script {

static CameraComponentPtr* cameraCreate(Entity& entity) {
	CameraComponentPtr* p = new CameraComponentPtr(new CameraComponent);
	entity.addComponent(p->get());
	return p;
}
static Vec3f* cameraGetVelocity(const CameraComponentPtr& c) {
	if(!c) return nullptr;
	return &c->velocity;
}
static void cameraSetVelocity(const CameraComponentPtr& c, const Vec3f& v) {
	if(!c) return;
	c->velocity = v;
}
SCRIPT_CLASS_REGISTER(CameraComponentPtr)
	.declareClass<CameraComponentPtr, ComponentPtr>(L"CameraComponent")
	.staticMethod<construct>(L"constructor", &cameraCreate)
	.wrappedMethod<objNoCare>(L"_getvelocity", &cameraGetVelocity)
	.wrappedMethod(L"_setvelocity", &cameraSetVelocity)
;}

}	// namespace script

namespace MCD {

void registerRenderBinding(script::VMCore* v)
{
	script::ClassTraits<CameraComponentPtr>::bind(v);
}

}	// namespace MCD
