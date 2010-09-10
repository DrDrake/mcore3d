#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"

using namespace MCD;

namespace {

struct Vec3
{
	Vec3() : x(0), y(0), z(0) {
		printf("a");
	}
	Vec3(float s) : x(s), y(s), z(s) {}
	Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	Vec3(const Vec3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}
	float x, y, z;
};

static int vec3Create(HSQUIRRELVM vm)
{
	using namespace MCD::Binding;

	const int paramCount = sq_gettop(vm) - 1;
	Vec3* v = nullptr;

	switch(paramCount) {
	case 0:
		v = new Vec3;	// Default construct
		break;
	case 1:
		if(sq_gettype(vm, 2) == OT_INSTANCE)
			v = new Vec3(get(TypeSelect<Vec3&>(), vm, 2));	// Copy construct
		else
			v = new Vec3(get(TypeSelect<float>(), vm, 2));	// Scalar construct
		break;
	case 3:
			v = new Vec3(
				get(TypeSelect<float>(), vm, 2),
				get(TypeSelect<float>(), vm, 3),
				get(TypeSelect<float>(), vm, 4)
			);
		break;
	default:
		return sq_throwerror(vm, "Mat44.constructor() wrong parameters");
	}

	// Pops the input params
	sq_pop(vm, paramCount);
	construct::pushResult(vm, v);
	return 1;
}

}	// namespace

namespace MCD {
namespace Binding {

SCRIPT_CLASS_DECLAR(Vec3);
SCRIPT_CLASS_REGISTER(Vec3)
	.declareClass<Vec3>("Vec3")
	.rawMethod("constructor", vec3Create)
;}

}	// namespace Binding
}   // namespace MCD

TEST(FunctionOverload_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Vec3>::bind(&vm);

	CHECK(vm.runScript("local v=Vec3()"));
	CHECK(vm.runScript("local v=Vec3(1)"));
	CHECK(vm.runScript("local v=Vec3(1,2,3)"));
}
