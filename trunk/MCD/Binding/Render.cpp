#include "Pch.h"
#include "Render.h"
#include "Binding.h"
#include "../Render/ChamferBox.h"
#include "../Render/Mesh.h"

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

SCRIPT_CLASS_REGISTER(Effect)
	.declareClass<Effect, Resource>(L"Effect")
;}

SCRIPT_CLASS_REGISTER(Mesh)
	.declareClass<Mesh, Resource>(L"Mesh")
;}

}	// namespace script

namespace MCD {

static Mesh* chamferBoxMeshCreate(float filletRadius, size_t filletSegmentCount)
{
	Mesh* mesh = new Mesh();
	ChamferBoxBuilder builder(filletRadius, filletSegmentCount);
	builder.commit(*mesh, MeshBuilder::Static);
	return mesh;
}

void registerRenderBinding(script::VMCore* v)
{
	using namespace script;
	script::ClassTraits<Effect>::bind(v);
	script::ClassTraits<Mesh>::bind(v);

	RootDeclarator root(v);
	root.declareFunction<objRefCount<resourceRefPolicy> >(L"ChamferBoxMesh", &chamferBoxMeshCreate);
}

}	// namespace MCD
