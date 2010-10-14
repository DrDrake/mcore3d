#include "Pch.h"
#include "CoreBindings.h"
#include "Declarator.h"
#include "VMCore.h"
#include "../Entity/Entity.h"

namespace MCD {

namespace Binding {

// Math

static int createMat44(HSQUIRRELVM vm)
{
	const int paramCount = sq_gettop(vm) - 1;
	Mat44f* m = nullptr;

	switch(paramCount) {
	case 0:
		m = new Mat44f(Mat44f::cIdentity);	// Default construct
		break;
	case 1:
		if(sq_gettype(vm, 2) == OT_INSTANCE)
			m = new Mat44f(get(TypeSelect<Mat44f&>(), vm, 2));	// Copy construct
		else
			m = new Mat44f(get(TypeSelect<float>(), vm, 2));	// Scalar construct
		break;
	case 3:
			#define GET(i) (get(TypeSelect<float>(), vm, i))
			m = new Mat44f(	// Element wise construct
				GET( 2), GET( 3), GET( 4), GET( 5),
				GET( 6), GET( 7), GET( 8), GET( 9),
				GET(10), GET(11), GET(12), GET(13),
				GET(14), GET(15), GET(16), GET(17)
			);
			#undef GET
		break;
	default:
		return sq_throwerror(vm, "Mat44f.constructor() get incorrect number of parameters");
	}

	// Pops the input params
	sq_pop(vm, paramCount);
	construct::pushResult(vm, m);
	return 1;
}

static float getAtMat44(const Mat44f& m, size_t i, size_t j) { return m[i][j]; }
static Mat44f addMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs + rhs; }
static Mat44f subMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs - rhs; }
static Mat44f mulMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs * rhs; }
static Mat44f scalarMulMat44(const Mat44f& lhs, float rhs) { return lhs * rhs; }
static bool isEqualMat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs == rhs; }
static bool isIdentityMat44(const Mat44f& m) { return m == Mat44f::cIdentity; }

SCRIPT_CLASS_REGISTER(Mat44f)
	.declareClass<Mat44f>("Mat44")
	.rawMethod("constructor", createMat44)
	.var("m00", (float Mat44f::*)&Mat44f::m00)	.var("m01", (float Mat44f::*)&Mat44f::m01)	.var("m02", (float Mat44f::*)&Mat44f::m02)	.var("m03", (float Mat44f::*)&Mat44f::m03)
	.var("m10", (float Mat44f::*)&Mat44f::m10)	.var("m11", (float Mat44f::*)&Mat44f::m11)	.var("m12", (float Mat44f::*)&Mat44f::m12)	.var("m13", (float Mat44f::*)&Mat44f::m13)
	.var("m20", (float Mat44f::*)&Mat44f::m20)	.var("m21", (float Mat44f::*)&Mat44f::m21)	.var("m22", (float Mat44f::*)&Mat44f::m22)	.var("m23", (float Mat44f::*)&Mat44f::m23)
	.var("m30", (float Mat44f::*)&Mat44f::m30)	.var("m31", (float Mat44f::*)&Mat44f::m31)	.var("m32", (float Mat44f::*)&Mat44f::m32)	.var("m33", (float Mat44f::*)&Mat44f::m33)
	.method("getAt", &getAtMat44)
	.varGet("transpose", (Mat44f (Mat44f::*)()const)&Mat44f::transpose)
	.varGet("determinant", &Mat44f::determinant)
	.varGet("inverse", (Mat44f (Mat44f::*)()const)&Mat44f::inverse)
	.varGet("xBiasVector", &Mat44f::xBiasVector)
	.varGet("yBiasVector", &Mat44f::yBiasVector)
	.varGet("zBiasVector", &Mat44f::zBiasVector)
	.varGet("translation", &Mat44f::translation)
	.varSet("translation", &Mat44f::setTranslation)
	.method("translateBy", &Mat44f::translateBy)
	.varGet("scale", &Mat44f::scale)
	.varSet("scale", &Mat44f::setScale)
	.method("scaleBy", &Mat44f::scaleBy)
	.method("setRotation", &Mat44f::setRotation)
	.method("rotateBy", &Mat44f::rotateBy)
	.staticMethod("makeAxisRotation", &Mat44f::makeAxisRotation)
	.method("_add", &addMat44)
	.method("_sub", &subMat44)
	.method("_mul", &mulMat44)
	.method("scalarMul", &scalarMulMat44)
	.method("isEqual", &isEqualMat44)
	.method("isIdentity", &isIdentityMat44)
//	.runScript("Mat44._tostring <- function(){return xBiasVector+\", \"+yBiasVector+\"), \"+zBiasVector;}")	// Vec3.tostring()
;}

static int createVec2(HSQUIRRELVM vm)
{
	const int paramCount = sq_gettop(vm) - 1;
	Vec2f* v = nullptr;

	switch(paramCount) {
	case 0:
		v = new Vec2f(Vec2f::cZero);	// Default construct
		break;
	case 1:
		if(sq_gettype(vm, 2) == OT_INSTANCE)
			v = new Vec2f(get(TypeSelect<Vec2f&>(), vm, 2));	// Copy construct
		else
			v = new Vec2f(get(TypeSelect<float>(), vm, 2));	// Scalar construct
		break;
	case 3:
			#define GET(i) (get(TypeSelect<float>(), vm, i))
			v = new Vec2f(GET(2), GET(3));	// Element wise construct
			#undef GET
		break;
	default:
		return sq_throwerror(vm, "Vec2f.constructor() get incorrect number of parameters");
	}

	// Pops the input params
	sq_pop(vm, paramCount);
	construct::pushResult(vm, v);
	return 1;
}

SCRIPT_CLASS_REGISTER(Vec2f)
	.declareClass<Vec2f>("Vec2")
	.rawMethod("constructor", createVec2)
	.var("x", (float Vec2f::*)&Vec2f::x)
	.var("y", (float Vec2f::*)&Vec2f::y)
	.runScript("Vec2._tostring <- function(){return x+\", \"+y+\";}")
;}

static int createVec3(HSQUIRRELVM vm)
{
	const int paramCount = sq_gettop(vm) - 1;
	Vec3f* v = nullptr;

	switch(paramCount) {
	case 0:
		v = new Vec3f(Vec3f::cZero);	// Default construct
		break;
	case 1:
		if(sq_gettype(vm, 2) == OT_INSTANCE)
			v = new Vec3f(get(TypeSelect<Vec3f&>(), vm, 2));	// Copy construct
		else
			v = new Vec3f(get(TypeSelect<float>(), vm, 2));	// Scalar construct
		break;
	case 3:
			#define GET(i) (get(TypeSelect<float>(), vm, i))
			v = new Vec3f(GET(2), GET(3), GET(4));	// Element wise construct
			#undef GET
		break;
	default:
		return sq_throwerror(vm, "Vec3f.constructor() get incorrect number of parameters");
	}

	// Pops the input params
	sq_pop(vm, paramCount);
	construct::pushResult(vm, v);
	return 1;
}

static float lengthVec3(const Vec3f& v) { return v.length(); }
static Vec3f addVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs + rhs; }
static Vec3f subVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs - rhs; }
static Vec3f mulVec3(const Vec3f& lhs, float rhs) { return lhs * rhs; }
static Vec3f unmVec3(const Vec3f& lhs) { return -lhs; }
static int cmpVec3(const Vec3f& lhs, const Vec3f& rhs) { if(lhs == rhs) return 0; return 1;/*return lhs > rhs ? 1 : -1;*/ }
static bool isEqualVec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs == rhs; }
static void addEqualVec3(Vec3f& lhs, const Vec3f& rhs) { lhs += rhs; }
static void mulEqualVec3(Vec3f& lhs, float rhs) { lhs *= rhs; }

SCRIPT_CLASS_REGISTER(Vec3f)
	.declareClass<Vec3f>("Vec3")
	.rawMethod("constructor", createVec3)
	.var("x", (float Vec3f::*)&Vec3f::x)
	.var("y", (float Vec3f::*)&Vec3f::y)
	.var("z", (float Vec3f::*)&Vec3f::z)
	.method("dot", &Vec3f::dot)
	.method("_modulo", &Vec3f::dot)
	.varGet("norm", &Vec3f::norm)
	.method("cross", &Vec3f::operator^)
	.varGet("length", &lengthVec3)	// Vec3f::length() didn't return float
	.varGet("squaredLength", &Vec3f::squaredLength)
	.method("distance", &Vec3f::distance)
	.method("squaredDistance", &Vec3f::squaredDistance)
	.method("normalize", &Vec3f::normalizeSafe)
	.method("_add", &addVec3)
	.method("_sub", &subVec3)
	.method("_mul", &mulVec3)
	.method("_unm", &unmVec3)
	.method("_cmp", &cmpVec3)
	.method("isEqual", &isEqualVec3)
	.method("addEqual", &addEqualVec3)
	.method("mulEqual", &mulEqualVec3)
	.runScript("Vec3._tostring <- function(){return x+\", \"+y+\", \"+z;}")
;}

// Entity

SCRIPT_CLASS_REGISTER(Entity)
	.declareClass<Entity>("Entity")
	.constructor("defaultConstructor")
	.runScript("Entity.constructor<-function(name=\"\"){defaultConstructor.call(this);this.name=name;}")
	.var("enabled", &Entity::enabled)
	.var("name", &Entity::name)
	.varGet("parent", (Entity* (Entity::*)())(&Entity::parent))
	.varGet("firstChild", (Entity* (Entity::*)())(&Entity::firstChild))
	.varGet("lastChild", (Entity* (Entity::*)())(&Entity::lastChild))
	.varGet("nextSibling", (Entity* (Entity::*)())(&Entity::nextSibling))
	.var("localTransform", &Entity::localTransform)
	.varGet("worldTransform", &Entity::worldTransform)
	.method("asChildOf", &Entity::asChildOf)
	.method("addFirstChild", &Entity::addFirstChild)
	.method("addLastChild", &Entity::addLastChild)
	.method("insertBefore", &Entity::insertBefore)
	.method("insertAfter", &Entity::insertAfter)
	.method("isAncestorOf", &Entity::isAncestorOf)
	.method("destroyThis", &Entity::destroyThis)
;}

void push(HSQUIRRELVM v, Entity* obj)
{
	if(!obj) {
		sq_pushnull(v);
		return;
	}

	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(obj->scriptHandle);

	// Use the stored script handle
	if(obj->scriptVm) {
		// TODO: Ensure the incomming vm is the same as the one stored in obj?
		sq_pushobject(v, *h);
	}
	else {
		// Get the actual class id from run-time type information
		const ClassID classID = ClassesManager::getClassIdFromRtti(
			typeid(*obj), ClassTraits<Entity>::classID()
		);

		obj->scriptAddReference();
		ClassesManager::createObjectInstanceOnStack(v, classID, obj);
		obj->scriptVm = v;
		sq_getstackobj(v, -1, h);
	}
}

SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity* instance)
{
	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(instance->scriptHandle);
	MCD_ASSUME(!instance->scriptVm);
	instance->scriptVm = v;
	sq_getstackobj(v, idx, h);

	instance->scriptAddReference();
	return sq_setinstanceup(v, idx, instance);
}

SQRESULT fromInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity*& instance, SQUserPointer typetag)
{
	return sq_getinstanceup(v, idx, (SQUserPointer*)&instance, typetag);
}

void destroy(Entity* dummy, Entity* instance)
{
	if(instance) {
		instance->scriptVm = nullptr;
		instance->scriptReleaseReference();
	}
}

void registerCoreBinding(VMCore& vm)
{
	Binding::ClassTraits<Mat44f>::bind(&vm);
	Binding::ClassTraits<Entity>::bind(&vm);
}

}	// namespace Binding

}	// namespace MCD
