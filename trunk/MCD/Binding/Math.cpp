#include "Pch.h"
#include "Math.h"
#include "Binding.h"

using namespace MCD;

namespace script {

static int mat44Create(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	int nparams = sa.getParamCount();

	Mat44f* m = nullptr;
	switch(nparams) {
	case 1:
		m = new Mat44f(Mat44f::cIdentity);	// Default construct
		break;
	case 2:
		if(sa.getType(2) == OT_INSTANCE)	// Copy construct
			m = new Mat44f(get(types::TypeSelect<Mat44f&>(), vm, 2));
		else
			m = new Mat44f(sa.getFloat(2));	// Scalar construct
		break;
	case 17:
			m = new Mat44f(
				sa.getFloat( 2), sa.getFloat( 3), sa.getFloat( 4), sa.getFloat( 5),
				sa.getFloat( 6), sa.getFloat( 7), sa.getFloat( 8), sa.getFloat( 9),
				sa.getFloat(10), sa.getFloat(11), sa.getFloat(12), sa.getFloat(13),
				sa.getFloat(14), sa.getFloat(15), sa.getFloat(16), sa.getFloat(17)
			);
		break;
	default:
		return sa.throwError(L"Mat44.constructor() wrong parameters");
	}

	// Pops the input params
	sq_pop(vm, nparams - 1);
	construct::pushResult(vm, m);
	return 1;
}

static Mat44f addMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs + rhs; }
static Mat44f subMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs - rhs; }
static Mat44f mulMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs * rhs; }
static Mat44f scalarMulMat44(const Mat44f& lhs, float rhs) { return lhs * rhs; }
static bool isEqualMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs == rhs; }

SCRIPT_CLASS_REGISTER_NAME(Mat44f, "Mat44")
	.enableGetset(L"Mat44")
	.rawMethod(L"constructor", mat44Create)
	.getset(L"m00", &Mat44f::m00)	.getset(L"m01", &Mat44f::m01)	.getset(L"m02", &Mat44f::m02)	.getset(L"m03", &Mat44f::m03)
	.getset(L"m10", &Mat44f::m10)	.getset(L"m11", &Mat44f::m11)	.getset(L"m12", &Mat44f::m12)	.getset(L"m13", &Mat44f::m13)
	.getset(L"m20", &Mat44f::m20)	.getset(L"m21", &Mat44f::m21)	.getset(L"m22", &Mat44f::m22)	.getset(L"m23", &Mat44f::m23)
	.getset(L"m30", &Mat44f::m30)	.getset(L"m31", &Mat44f::m31)	.getset(L"m32", &Mat44f::m32)	.getset(L"m33", &Mat44f::m33)
	.method(L"_gettranspose", (Mat44f (Mat44f::*)()const)&Mat44f::transpose)
	.method(L"_getdeterminant", &Mat44f::determinant)
	.method(L"_getinverse", (Mat44f (Mat44f::*)()const)&Mat44f::inverse)
	.method(L"_getxBiasVector", &Mat44f::xBiasVector)
	.method(L"_getyBiasVector", &Mat44f::yBiasVector)
	.method(L"_getzBiasVector", &Mat44f::zBiasVector)
	.method(L"_gettranslation", &Mat44f::translation)
	.method(L"_settranslation", &Mat44f::setTranslation)
	.method(L"translateBy", &Mat44f::translateBy)
	.method(L"_getscale", &Mat44f::scale)
	.method(L"_setscale", &Mat44f::setScale)
	.method(L"scaleBy", &Mat44f::scaleBy)
	.staticMethod(L"makeAxisRotation", &Mat44f::makeAxisRotation)
	.wrappedMethod(L"_add", &addMat44)
	.wrappedMethod(L"_sub", &subMat44)
	.wrappedMethod(L"_mul", &mulMat44)
	.wrappedMethod(L"scalarMul", &scalarMulMat44)
//	.wrappedMethod(L"scalarDiv", &scalarDivMat44)
	.wrappedMethod(L"isEqual", &isEqualMat44)
//	.runScript(L"Mat44.identity <- Mat44()")	// TODO: Add identity
	.runScript(L"Mat44._tostring <- function(){return xBiasVector+\", \"+yBiasVector+\", \"+zBiasVector;}")	// Vec3.tostring()
;}

static int vec2Create(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	int nparams = sa.getParamCount();

	Vec2f* v = nullptr;
	switch(nparams) {
	case 1:
		v = new Vec2f(0.0f);	// Default construct
		break;
	case 2:
		if(sa.getType(2) == OT_INSTANCE)	// Copy construct
			v = new Vec2f(get(types::TypeSelect<Vec2f&>(), vm, 2));
		else
			v = new Vec2f(sa.getFloat(2));	// Scalar construct
		break;
	case 3:
		v = new Vec2f(sa.getFloat(2), sa.getFloat(3));	// Element-wise construct
		break;
	default:
		return sa.throwError(L"Vec2.constructor() wrong parameters");
	}

	// Pops the input params
	sq_pop(vm, nparams - 1);
	construct::pushResult(vm, v);
	return 1;
}
static Vec2f addVec2(const Vec2f& lhs, const Vec2f& rhs) { return lhs + rhs; }
static Vec2f subVec2(const Vec2f& lhs, const Vec2f& rhs) { return lhs - rhs; }
static bool isEqualVec2(const Vec2f& lhs, const Vec2f& rhs) { return lhs == rhs; }
static void vec2MulEqual(Vec2f& lhs, float rhs) { lhs *= rhs; }

SCRIPT_CLASS_REGISTER_NAME(Vec2f, "Vec2")
	.enableGetset(L"Vec2")
	.rawMethod(L"constructor", vec2Create)
	.getset(L"x", &Vec2f::x)
	.getset(L"y", &Vec2f::y)
	.wrappedMethod(L"_add", &addVec2)
	.wrappedMethod(L"_sub", &subVec2)
	.wrappedMethod(L"isEqual", &isEqualVec2)
	.wrappedMethod(L"mulEqual", &vec2MulEqual)
	.runScript(L"Vec2._tostring <- function(){return x+\", \"+y;}")	// Vec2.tostring()
;}

static int vec3Create(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	int nparams = sa.getParamCount();

	Vec3f* v = nullptr;
	switch(nparams) {
	case 1:
		v = new Vec3f(0.0f);	// Default construct
		break;
	case 2:
		if(sa.getType(2) == OT_INSTANCE)	// Copy construct
			v = new Vec3f(get(types::TypeSelect<Vec3f&>(), vm, 2));
		else
			v = new Vec3f(sa.getFloat(2));	// Scalar construct
		break;
	case 4:
		v = new Vec3f(sa.getFloat(2), sa.getFloat(3), sa.getFloat(4));	// Element-wise construct
		break;
	default:
		return sa.throwError(L"Vec3.constructor() wrong parameters");
	}

	// Pops the input params
	sq_pop(vm, nparams - 1);
	construct::pushResult(vm, v);
	return 1;
}
static float vec3Length(const Vec3f& v) { return v.length(); }
static Vec3f addVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs + rhs; }
static Vec3f subVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs - rhs; }
static Vec3f mulVec3(const Vec3f& lhs, float rhs) { return lhs * rhs; }
static Vec3f unmVec3(const Vec3f& lhs) { return -lhs; }
static bool isEqualVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs == rhs; }
static void vec3AddEqual(Vec3f& lhs, const Vec3f& rhs) { lhs += rhs; }
static void vec3MulEqual(Vec3f& lhs, float rhs) { lhs *= rhs; }
static int cmpVec3(const Vec3f& lhs, const Vec3f& rhs) {
	if(lhs == rhs) return 0;
	return 1;
//	return lhs > rhs ? 1 : -1;
}

SCRIPT_CLASS_REGISTER_NAME(Vec3f, "Vec3")
	.enableGetset(L"Vec3")
	.rawMethod(L"constructor", vec3Create)
	.method(L"dot", &Vec3f::dot)
	.method(L"_modulo", &Vec3f::dot)
	.method(L"_getnorm", &Vec3f::norm)
	.method(L"cross", &Vec3f::operator^)
	.wrappedMethod(L"_getlength", &vec3Length)	// Vec3f::length() didn't return float
	.method(L"_getsquaredLength", &Vec3f::squaredLength)
	.method(L"distance", &Vec3f::distance)
	.method(L"squaredDistance", &Vec3f::squaredDistance)
	.method(L"normalize", &Vec3f::normalizeSafe)
	.getset(L"x", &Vec3f::x)
	.getset(L"y", &Vec3f::y)
	.getset(L"z", &Vec3f::z)
	.wrappedMethod(L"_add", &addVec3)
	.wrappedMethod(L"_sub", &subVec3)
	.wrappedMethod(L"_mul", &mulVec3)
	.wrappedMethod(L"_unm", &unmVec3)
	.wrappedMethod(L"isEqual", &isEqualVec3)
	.wrappedMethod(L"_cmp", &cmpVec3)
	.wrappedMethod(L"addEqual", &vec3AddEqual)
	.wrappedMethod(L"mulEqual", &vec3MulEqual)
	.runScript(L"Vec3._tostring <- function(){return x+\", \"+y+\", \"+z;}")	// Vec3.tostring()
;}

}	// namespace script

namespace MCD {

void registerMathBinding(script::VMCore* v)
{
	script::ClassTraits<Mat44f>::bind(v);
	script::ClassTraits<Vec2f>::bind(v);
	script::ClassTraits<Vec3f>::bind(v);
}

}	// namespace MCD
