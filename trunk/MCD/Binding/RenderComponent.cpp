#include "Pch.h"
#include "RenderComponent.h"
#include "Binding.h"
#include "Math.h"
#include "Render.h"

using namespace MCD;

namespace script {

struct resourceRefPolicy {
	static void addRef(Resource* resource) {
		intrusivePtrAddRef(resource);
	}
	static void releaseRef(Resource* resource) {
		intrusivePtrRelease(resource);
	}
};	// resourceRefPolicy


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

SCRIPT_CLASS_REGISTER(MeshComponent)
	.declareClass<MeshComponent, Component>(L"MeshComponent")
	.enableGetset(L"MeshComponent")
	.constructor()
	.wrappedMethod<objRefCount<resourceRefPolicy> >(L"_getmesh", &meshComponentGetMesh)
	.wrappedMethod(L"_setmesh", &meshComponentSetMesh)
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
