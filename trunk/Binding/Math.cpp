#include "Pch.h"
#include "Math.h"
#include "Binding.h"

using namespace MCD;

namespace script {

static Mat44f addMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs + rhs; }
static Mat44f subMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs - rhs; }

SCRIPT_CLASS_REGISTER_NAME(Mat44f, "Mat44")
	.constructor(L"create")
	.method(L"_gettranspose", (Mat44f (Mat44f::*)()const)&Mat44f::transpose)
	.method(L"_getdeterminant", &Mat44f::determinant)
	.method(L"_getinverse", (Mat44f (Mat44f::*)()const)&Mat44f::inverse)
	.wrappedMethod(L"_add", &addMat44)
	.wrappedMethod(L"_sub", &subMat44)
;}

static float vec3Length(const Vec3f& v) { return v.length(); }
static Vec3f* addVec3(const Vec3f& lhs, const Vec3f& rhs) { return new Vec3f(lhs + rhs); }
static Vec3f subVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs - rhs; }
static Vec3f* mulVec3(const Vec3f& lhs, float rhs) { return new Vec3f(lhs * rhs); }
static Vec3f unmVec3(const Vec3f& lhs) { return -lhs; }
static bool isEqualVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs == rhs; }
static int cmpVec3(const Vec3f& lhs, const Vec3f& rhs) {
	if(lhs == rhs) return 0;
	return 1;
//	return lhs > rhs ? 1 : -1;
}
static void addEqual(Vec3f& lhs, const Vec3f& rhs) {
	lhs += rhs;
}
static void mulEqual(Vec3f& lhs, float rhs) {
	lhs *= rhs;
}

SCRIPT_CLASS_REGISTER_NAME(Vec3f, "Vec3")
	.constructor(L"create")
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
	.wrappedMethod(L"addEqual", &addEqual)
	.wrappedMethod(L"mulEqual", &mulEqual)
;}

}	// namespace script

namespace MCD {

void registerMathBinding(script::VMCore* v)
{
	script::ClassTraits<Mat44f>::bind(v);
	script::ClassTraits<Vec3f>::bind(v);
}

}	// namespace MCD
