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
		return sa.throwError("Mat44.constructor() wrong parameters");
	}

	// Pops the input params
	sq_pop(vm, nparams - 1);
	construct::pushResult(vm, m);
	return 1;
}

static float mat44GetAt(const Mat44f& m, size_t i, size_t j) { return m[i][j]; }
static Mat44f addMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs + rhs; }
static Mat44f subMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs - rhs; }
static Mat44f mulMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs * rhs; }
static Mat44f scalarMulMat44(const Mat44f& lhs, float rhs) { return lhs * rhs; }
static bool isEqualMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs == rhs; }
static bool mat44IsIdentity(const Mat44f& m) { return m == Mat44f::cIdentity; }
static void mat44FromHex(Mat44f& m, const char* s) {
	MCD_ASSERT(::strlen(s) == sizeof(float) * 2 * (4*4));
	MCD_VERIFY(::sscanf(s, "%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X%8X",
		(int*)&m.m00, (int*)&m.m01, (int*)&m.m02, (int*)&m.m03,
		(int*)&m.m10, (int*)&m.m11, (int*)&m.m12, (int*)&m.m13,
		(int*)&m.m20, (int*)&m.m21, (int*)&m.m22, (int*)&m.m23,
		(int*)&m.m30, (int*)&m.m31, (int*)&m.m32, (int*)&m.m33
	) == 16);
}

SCRIPT_CLASS_REGISTER_NAME(Mat44f, "Mat44")
	.enableGetset()
	.rawMethod(xSTRING("constructor"), mat44Create)
	.getset(xSTRING("m00"), &Mat44f::m00)	.getset(xSTRING("m01"), &Mat44f::m01)	.getset(xSTRING("m02"), &Mat44f::m02)	.getset(xSTRING("m03"), &Mat44f::m03)
	.getset(xSTRING("m10"), &Mat44f::m10)	.getset(xSTRING("m11"), &Mat44f::m11)	.getset(xSTRING("m12"), &Mat44f::m12)	.getset(xSTRING("m13"), &Mat44f::m13)
	.getset(xSTRING("m20"), &Mat44f::m20)	.getset(xSTRING("m21"), &Mat44f::m21)	.getset(xSTRING("m22"), &Mat44f::m22)	.getset(xSTRING("m23"), &Mat44f::m23)
	.getset(xSTRING("m30"), &Mat44f::m30)	.getset(xSTRING("m31"), &Mat44f::m31)	.getset(xSTRING("m32"), &Mat44f::m32)	.getset(xSTRING("m33"), &Mat44f::m33)
	.wrappedMethod(xSTRING("getAt"), &mat44GetAt)
	.method(xSTRING("_gettranspose"), (Mat44f (Mat44f::*)()const)&Mat44f::transpose)
	.method(xSTRING("_getdeterminant"), &Mat44f::determinant)
	.method(xSTRING("_getinverse"), (Mat44f (Mat44f::*)()const)&Mat44f::inverse)
	.method(xSTRING("_getxBiasVector"), &Mat44f::xBiasVector)
	.method(xSTRING("_getyBiasVector"), &Mat44f::yBiasVector)
	.method(xSTRING("_getzBiasVector"), &Mat44f::zBiasVector)
	.method(xSTRING("_gettranslation"), &Mat44f::translation)
	.method(xSTRING("_settranslation"), &Mat44f::setTranslation)
	.method(xSTRING("translateBy"), &Mat44f::translateBy)
	.method(xSTRING("_getscale"), &Mat44f::scale)
	.method(xSTRING("_setscale"), &Mat44f::setScale)
	.method(xSTRING("scaleBy"), &Mat44f::scaleBy)
	.staticMethod(xSTRING("makeAxisRotation"), &Mat44f::makeAxisRotation)
	.wrappedMethod(xSTRING("_add"), &addMat44)
	.wrappedMethod(xSTRING("_sub"), &subMat44)
	.wrappedMethod(xSTRING("_mul"), &mulMat44)
	.wrappedMethod(xSTRING("scalarMul"), &scalarMulMat44)
//	.wrappedMethod(xSTRING("scalarDiv"), &scalarDivMat44)
	.wrappedMethod(xSTRING("isEqual"), &isEqualMat44)
	.wrappedMethod(xSTRING("isIdentity"), &mat44IsIdentity)
	.wrappedMethod(xSTRING("fromHex"), &mat44FromHex)
	.runScript(xSTRING("Mat44.toHex <- function(){local str=\"\"; for(local i=0;i<4;++i) for(local j=0;j<4;++j) str+=::floatToHex(getAt(i,j)); return str;}"))
	.runScript(xSTRING("Mat44._tostring <- function(){return xBiasVector+\"), \"+yBiasVector+\"), \"+zBiasVector;}"))	// Vec3.tostring()
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
		return sa.throwError("Vec2.constructor() wrong parameters");
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
static void vec2FromHex(Vec2f& v, const char* s) {
	MCD_ASSERT(::strlen(s) == sizeof(float) * 2 * 2);
	MCD_VERIFY(::sscanf(s, "%X%X", (int*)&v.x, (int*)&v.y) == 2);
}

SCRIPT_CLASS_REGISTER_NAME(Vec2f, "Vec2")
	.enableGetset()
	.rawMethod(xSTRING("constructor"), vec2Create)
	.getset(xSTRING("x"), &Vec2f::x)
	.getset(xSTRING("y"), &Vec2f::y)
	.wrappedMethod(xSTRING("_add"), &addVec2)
	.wrappedMethod(xSTRING("_sub"), &subVec2)
	.wrappedMethod(xSTRING("isEqual"), &isEqualVec2)
	.wrappedMethod(xSTRING("mulEqual"), &vec2MulEqual)
	.wrappedMethod(xSTRING("fromHex"), &vec2FromHex)
	.runScript(xSTRING("Vec2.toHex <- function(){return ::floatToHex(x)+::floatToHex(y);}"))
	.runScript(xSTRING("Vec2._tostring <- function(){return x+\", \"+y;}"))	// Vec2.tostring()
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
		return sa.throwError("Vec3.constructor() wrong parameters");
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
static int cmpVec3(const Vec3f& lhs, const Vec3f& rhs) {
	if(lhs == rhs) return 0;
	return 1;
//	return lhs > rhs ? 1 : -1;
}
static void vec3AddEqual(Vec3f& lhs, const Vec3f& rhs) { lhs += rhs; }
static void vec3MulEqual(Vec3f& lhs, float rhs) { lhs *= rhs; }
static void vec3FromHex(Vec3f& v, const char* s) {
	MCD_ASSERT(::strlen(s) == sizeof(float) * 2 * 3);
	MCD_VERIFY(::sscanf(s, "%X%X%X", (int*)&v.x, (int*)&v.y, (int*)&v.z) == 3);
}

SCRIPT_CLASS_REGISTER_NAME(Vec3f, "Vec3")
	.enableGetset()
	.rawMethod(xSTRING("constructor"), vec3Create)
	.method(xSTRING("dot"), &Vec3f::dot)
	.method(xSTRING("_modulo"), &Vec3f::dot)
	.method(xSTRING("_getnorm"), &Vec3f::norm)
	.method(xSTRING("cross"), &Vec3f::operator^)
	.wrappedMethod(xSTRING("_getlength"), &vec3Length)	// Vec3f::length() didn't return float
	.method(xSTRING("_getsquaredLength"), &Vec3f::squaredLength)
	.method(xSTRING("distance"), &Vec3f::distance)
	.method(xSTRING("squaredDistance"), &Vec3f::squaredDistance)
	.method(xSTRING("normalize"), &Vec3f::normalizeSafe)
	.getset(xSTRING("x"), &Vec3f::x)
	.getset(xSTRING("y"), &Vec3f::y)
	.getset(xSTRING("z"), &Vec3f::z)
	.wrappedMethod(xSTRING("_add"), &addVec3)
	.wrappedMethod(xSTRING("_sub"), &subVec3)
	.wrappedMethod(xSTRING("_mul"), &mulVec3)
	.wrappedMethod(xSTRING("_unm"), &unmVec3)
	.wrappedMethod(xSTRING("isEqual"), &isEqualVec3)
	.wrappedMethod(xSTRING("_cmp"), &cmpVec3)
	.wrappedMethod(xSTRING("addEqual"), &vec3AddEqual)
	.wrappedMethod(xSTRING("mulEqual"), &vec3MulEqual)
	.wrappedMethod(xSTRING("fromHex"), &vec3FromHex)
	.runScript(xSTRING("Vec3.toHex <- function(){return ::floatToHex(x)+::floatToHex(y)+::floatToHex(z);}"))
	.runScript(xSTRING("Vec3._tostring <- function(){return x+\", \"+y+\", \"+z;}"))	// Vec3.tostring()
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
