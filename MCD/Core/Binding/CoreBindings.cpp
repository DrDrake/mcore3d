#include "Pch.h"
#include "CoreBindings.h"
#include "Declarator.h"
#include "ScriptComponent.h"
#include "VMCore.h"
#include "../Entity/InputComponent.h"
#include "../Entity/Entity.h"
#include "../System/Resource.h"
#include "../System/ResourceManager.h"
#include "../System/Timer.h"

namespace MCD {
namespace Binding {

// System

static const char* path_Resource(Resource& self) { return self.fileId().c_str(); }

SCRIPT_CLASS_REGISTER_NAME(Resource)
	.varGet("path", &path_Resource)
	.varGet("commitCount", &Resource::commitCount)
	.method("_tostring", &path_Resource)
;}

void destroy(Resource* p, Resource*)
{
	if(p)
		intrusivePtrRelease(p);
}

void push(HSQUIRRELVM v, Resource* obj, Resource*)
{
	if(!obj) {
		sq_pushnull(v);
		return;
	}

	// Get the actual class id from run-time type information
	const ClassID classID = ClassesManager::getClassIdFromRtti(
		typeid(*obj), ClassTraits<Resource>::classID()
	);

	intrusivePtrAddRef(obj);
	ClassesManager::createObjectInstanceOnStack(v, classID, obj);
}

static Resource* load_ResourceManager(ResourceManager& self, const char* fileId, int blockIteration, uint priority, const char* args) {
	// TODO: Fix the blocking option
	return self.load(fileId, blockIteration, priority, args).get();
}

SCRIPT_CLASS_REGISTER_NAME(ResourceManager)
	.method("load", &load_ResourceManager)
;}

static float get_Timer(Timer& self) { return float(self.get().asSecond()); }
static float reset_Timer(Timer& self) { return float(self.reset().asSecond()); }

SCRIPT_CLASS_REGISTER_NAME(Timer)
	.constructor()
	.method("get", &get_Timer)
	.method("reset", &reset_Timer)
;}

// Math

static SQInteger create_Mat44(HSQUIRRELVM vm)
{
	const SQInteger paramCount = sq_gettop(vm) - 1;
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

static float getAt_Mat44(const Mat44f& m, size_t i, size_t j) { return m[i][j]; }
static Mat44f add_Mat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs + rhs; }
static Mat44f sub_Mat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs - rhs; }
static Mat44f mul_Mat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs * rhs; }
static Mat44f scalarMul_Mat44(const Mat44f& lhs, float rhs) { return lhs * rhs; }
static bool isEqual_Mat44(const Mat44f& lhs, const Mat44f& rhs) { return lhs == rhs; }
static bool isIdentity_Mat44(const Mat44f& m) { return m == Mat44f::cIdentity; }

SCRIPT_CLASS_REGISTER(Mat44f)
	.declareClass<Mat44f>("Mat44")
	.rawMethod("constructor", create_Mat44)
	.var("m00", (float Mat44f::*)&Mat44f::m00)	.var("m01", (float Mat44f::*)&Mat44f::m01)	.var("m02", (float Mat44f::*)&Mat44f::m02)	.var("m03", (float Mat44f::*)&Mat44f::m03)
	.var("m10", (float Mat44f::*)&Mat44f::m10)	.var("m11", (float Mat44f::*)&Mat44f::m11)	.var("m12", (float Mat44f::*)&Mat44f::m12)	.var("m13", (float Mat44f::*)&Mat44f::m13)
	.var("m20", (float Mat44f::*)&Mat44f::m20)	.var("m21", (float Mat44f::*)&Mat44f::m21)	.var("m22", (float Mat44f::*)&Mat44f::m22)	.var("m23", (float Mat44f::*)&Mat44f::m23)
	.var("m30", (float Mat44f::*)&Mat44f::m30)	.var("m31", (float Mat44f::*)&Mat44f::m31)	.var("m32", (float Mat44f::*)&Mat44f::m32)	.var("m33", (float Mat44f::*)&Mat44f::m33)
	.method("getAt", &getAt_Mat44)
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
	.method("_add", &add_Mat44)
	.method("_sub", &sub_Mat44)
	.method("_mul", &mul_Mat44)
	.method("scalarMul", &scalarMul_Mat44)
	.method("isEqual", &isEqual_Mat44)
	.method("isIdentity", &isIdentity_Mat44)
	.runScript("Mat44._tostring <- function(){return xBiasVector+\", \"+yBiasVector+\"), \"+zBiasVector;}")	// Mat44.tostring()
;}

static SQInteger create_Vec2(HSQUIRRELVM vm)
{
	const SQInteger paramCount = sq_gettop(vm) - 1;
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
	case 2:
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
	.rawMethod("constructor", create_Vec2)
	.var("x", (float Vec2f::*)&Vec2f::x)
	.var("y", (float Vec2f::*)&Vec2f::y)
	.runScript("Vec2._tostring <- function(){return x+\", \"+y;}")
;}

static SQInteger create_Vec3(HSQUIRRELVM vm)
{
	const SQInteger paramCount = sq_gettop(vm) - 1;
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

static float length_Vec3(const Vec3f& v) { return v.length(); }
static Vec3f add_Vec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs + rhs; }
static Vec3f sub_Vec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs - rhs; }
static Vec3f mul_Vec3(const Vec3f& lhs, float rhs) { return lhs * rhs; }
static Vec3f unm_Vec3(const Vec3f& lhs) { return -lhs; }
static int cmp_Vec3(const Vec3f& lhs, const Vec3f& rhs) { if(lhs == rhs) return 0; return 1;/*return lhs > rhs ? 1 : -1;*/ }
static bool isEqual_Vec3(const Vec3f& lhs, const Vec3f& rhs) { return lhs == rhs; }
static void addEqual_Vec3(Vec3f& lhs, const Vec3f& rhs) { lhs += rhs; }
static void mulEqual_Vec3(Vec3f& lhs, float rhs) { lhs *= rhs; }

SCRIPT_CLASS_REGISTER(Vec3f)
	.declareClass<Vec3f>("Vec3")
	.rawMethod("constructor", create_Vec3)
	.var("x", (float Vec3f::*)&Vec3f::x)
	.var("y", (float Vec3f::*)&Vec3f::y)
	.var("z", (float Vec3f::*)&Vec3f::z)
	.method("dot", &Vec3f::dot)
	.method("_modulo", &Vec3f::dot)
	.varGet("norm", &Vec3f::norm)
	.method("cross", &Vec3f::operator^)
	.varGet("length", &length_Vec3)	// Vec3f::length() didn't return float
	.varGet("squaredLength", &Vec3f::squaredLength)
	.method("distance", &Vec3f::distance)
	.method("squaredDistance", &Vec3f::squaredDistance)
	.method("normalize", &Vec3f::normalizeSafe)
	.method("_add", &add_Vec3)
	.method("_sub", &sub_Vec3)
	.method("_mul", &mul_Vec3)
	.method("_unm", &unm_Vec3)
	.method("_cmp", &cmp_Vec3)
	.method("isEqual", &isEqual_Vec3)
	.method("addEqual", &addEqual_Vec3)
	.method("mulEqual", &mulEqual_Vec3)
	.runScript("Vec3._tostring <- function(){return x+\", \"+y+\", \"+z;}")
;}

// Entity

static Component* nextComponent_Entity(Entity& self, Component* c) {
	c = (c == nullptr) ? self.components.begin() : c->next();
	return c == self.components.end() ? nullptr : c;
}

SCRIPT_CLASS_REGISTER_NAME(Entity)
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
	.varSet("worldTransform", &Entity::setWorldTransform)
	.method("asChildOf", (void (Entity::*)(Entity*))(&Entity::asChildOf))
	.method("_addFirstChild", (Entity* (Entity::*)(Entity*))(&Entity::addFirstChild))
	.runScript("Entity.addFirstChild<-function(arg){if(arg instanceof Entity)return _addFirstChild(arg);else return _addFirstChild(Entity(arg));}")
	.method("_addLastChild", (Entity* (Entity::*)(Entity*))(&Entity::addLastChild))
	.runScript("Entity.addLastChild<-function(arg){if(arg instanceof Entity)return _addLastChild(arg);else return _addLastChild(Entity(arg));}")
	.method("insertBefore", (void (Entity::*)(Entity*))(&Entity::insertBefore))
	.method("insertAfter", (void (Entity::*)(Entity*))(&Entity::insertAfter))
	.method("isAncestorOf", &Entity::isAncestorOf)
	.method("destroyThis", &Entity::destroyThis)
	.method("findEntityByPath", &Entity::findEntityByPath)
	.method("getRelativePathFrom", &Entity::getRelativePathFrom)
	.method("_addComponent", &Entity::_addComponent)
	.runScript("Entity.addComponent<-function(arg){if(arg instanceof ::Component) return _addComponent(arg);else if(typeof arg==\"class\") return _addComponent(arg());}") // Accept both class and instance as the argument
	.method("_nextComponent", &nextComponent_Entity)
	.runScript("Entity.hasComponent<-function(componentClass){for(local c;c=_nextComponent(c);){if(getroottable()[typeof c]==componentClass)return true;}return false;}")
	.runScript("Entity.__getTable.components<-function(){for(local c;c=_nextComponent(c);)yield c;}")	// Variable for looping all the components
//	.runScript("Entity._get<-function(idx){local t=::Entity.__getTable;if(t.rawin(idx))return t.rawget(idx).call(this);foreach(c in this.components)if(typeof c==idx)return c;throw null;}")	// NOTE: Using the generator here will cause crash in ~SQGenerator(), don't know why
	.runScript("Entity._get<-function(idx){local t=::Entity.__getTable;if(t.rawin(idx))return t.rawget(idx).call(this);for(local c;c=_nextComponent(c);)if(typeof c==idx)return c;throw null;}")	// Custom _get() to return component
	.runScript("Entity.__getTable.descendants<-function() {yield this;local e=firstChild;while(e){foreach(e2 in e.descendants){yield e2;}e=e.nextSibling;}}")
;}

void destroy(Entity* p, Entity*)
{
	if(p) {
		p->scriptVm = nullptr;
		p->scriptReleaseReference();
	}
}

void push(HSQUIRRELVM v, Entity* obj, Entity*)
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
		const ClassID classID = ClassTraits<Entity>::classID();
		intrusivePtrAddRef(obj);
		ClassesManager::createObjectInstanceOnStack(v, classID, obj);
		obj->scriptVm = v;
		sq_getstackobj(v, -1, h);
	}
}

SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Resource* p, Resource*)
{
	return sq_setinstanceup(v, idx, p);
}

SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* instance, Entity*)
{
	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(instance->scriptHandle);
	MCD_ASSUME(!instance->scriptVm);
	instance->scriptVm = v;
	sq_getstackobj(v, idx, h);

	intrusivePtrAddRef(instance);
	return sq_setinstanceup(v, idx, instance);
}

// Component

SCRIPT_CLASS_REGISTER_NAME(Component)
	.varGet("enabled", &Component::enabled)
	.varSet("enabled", &Component::setEnabled)
	.varGet("entity", &Component::entity)
	.method("destroyThis", &Component::destroyThis)
;}

void destroy(Component* p, Component*)
{
	if(p) {
		p->scriptVm = nullptr;
		intrusivePtrRelease(p);
	}
}

void push(HSQUIRRELVM v, Component* obj, Component*)
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
			typeid(*obj), ClassTraits<Component>::classID()
		);

		intrusivePtrAddRef(obj);
		ClassesManager::createObjectInstanceOnStack(v, classID, obj);
		obj->scriptVm = v;
		sq_getstackobj(v, -1, h);
	}
}

SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Component* instance, Component*)
{
	HSQOBJECT* h = reinterpret_cast<HSQOBJECT*>(instance->scriptHandle);
	MCD_ASSUME(!instance->scriptVm);
	instance->scriptVm = v;
	sq_getstackobj(v, idx, h);

	intrusivePtrAddRef(instance);
	return sq_setinstanceup(v, idx, instance);
}

// ScriptComponent

SQInteger suspend_ScriptComponent(HSQUIRRELVM vm) {
	return sq_suspendvm(vm);
}

SQInteger wakeup_ScriptComponent(HSQUIRRELVM vm)
{
	const SQInteger paramCount = sq_gettop(vm) - 1;

	SQRESULT ret = sq_wakeupvm(vm, paramCount > 0, true, true, false);

	if(SQ_VMSTATE_SUSPENDED != sq_getvmstate(vm)) {
		VMCore* core = reinterpret_cast<VMCore*>(sq_getforeignptr(vm));
		MCD_ASSUME(core);
		core->releaseThread(vm);

		ScriptComponent* self = get(TypeSelect<ScriptComponent*>(), vm, 1);
		MCD_ASSUME(self);
		self->mSuspended = false;
	}

	return ret;
}

SQInteger sleep_ScriptComponent(HSQUIRRELVM vm)
{
	VMCore* core = reinterpret_cast<VMCore*>(sq_getforeignptr(vm));
	MCD_ASSUME(core);
	const SQInteger paramCount = sq_gettop(vm) - 1;
	if(paramCount >= 1) {
		float t = 0;
		if(SQ_SUCCEEDED(sq_getfloat(vm, 2, &t))) {
			ScriptComponent* self = get(TypeSelect<ScriptComponent*>(), vm, 1);
			core->scheduleWakeup(vm, core->currentTime() + t, self);
			return sq_suspendvm(vm);
		}
	}

	return sq_throwerror(vm, "ScriptComponent.sleep() expecting a float parameter");
}

SCRIPT_CLASS_DECLAR_EXPORT(ScriptComponent, MCD_CORE_API);	// TODO: Don't why this is needed!
SCRIPT_CLASS_REGISTER(ScriptComponent)
	.declareClass<ScriptComponent, Component>("ScriptComponent")
	.constructor()
	.rawMethod("suspend", &suspend_ScriptComponent)
	.rawMethod("wakeup", &wakeup_ScriptComponent)
	.rawMethod("sleep", &sleep_ScriptComponent)
;}

// Input

static Vec2f getMousePosition_InputComponent(InputComponent& self)
{
	// Transform the result from type Vec2i to Vec2f
	Vec2i pos = self.getMousePosition();
	return Vec2f(float(pos.x), float(pos.y));
}

SCRIPT_CLASS_REGISTER(InputComponent)
	.declareClass<InputComponent, Component>("InputComponent")
	.varGet("anyKey", &InputComponent::anyKey)
	.varGet("anyKeyDown", &InputComponent::anyKeyDown)
	.method("getAxis", &InputComponent::getAxis)
	.method("getAxisRaw", &InputComponent::getAxisRaw)
	.method("getAxisDelta", &InputComponent::getAxisDelta)
	.method("getAxisDeltaRaw", &InputComponent::getAxisDeltaRaw)
	.method("getButton", &InputComponent::getButton)
	.method("getButtonDown", &InputComponent::getButtonDown)
	.method("getButtonUp", &InputComponent::getButtonUp)
	.varGet("mousePosition", &getMousePosition_InputComponent)
	.method("getMouseButton", &InputComponent::getMouseButton)
	.method("getMouseButtonDown", &InputComponent::getMouseButtonDown)
	.method("getMouseButtonUp", &InputComponent::getMouseButtonUp)
	.varGet("inputString", &InputComponent::inputString)
;}

void registerCoreBinding(VMCore& vm)
{
	Binding::ClassTraits<Component>::bind(&vm);
	Binding::ClassTraits<Entity>::bind(&vm);
	Binding::ClassTraits<InputComponent>::bind(&vm);
	Binding::ClassTraits<Mat44f>::bind(&vm);
	Binding::ClassTraits<Resource>::bind(&vm);
	Binding::ClassTraits<ResourceManager>::bind(&vm);
	Binding::ClassTraits<ScriptComponent>::bind(&vm);
	Binding::ClassTraits<Timer>::bind(&vm);
	Binding::ClassTraits<Vec2f>::bind(&vm);
	Binding::ClassTraits<Vec3f>::bind(&vm);
}

}	// namespace Binding
}	// namespace MCD
