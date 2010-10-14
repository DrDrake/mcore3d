#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"
#include "../../../MCD/Core/Entity/Entity.h"

using namespace MCD;

namespace MCD {
namespace Binding {

void push(HSQUIRRELVM v, Entity* obj);
SQRESULT setInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity* instance);
SQRESULT fromInstanceUp(HSQUIRRELVM v, SQInteger idx, Entity* dummy, Entity*& instance, SQUserPointer typetag);
void destroy(Entity*, Entity*);

SCRIPT_CLASS_DECLAR(Entity);
SCRIPT_CLASS_REGISTER(Entity)
	.declareClass<Entity>("Entity")
	.constructor()
	.var("name", &Entity::name)
	.method("asChildOf", &Entity::asChildOf)
	.method("parent", (Entity* (Entity::*)())(&Entity::parent))
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

}	// namespace Binding
}	// namespace MCD

TEST(Entity_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Entity>::bind(&vm);

	CHECK(vm.runScript(
		"local e1 = Entity();\n"
		"local e2 = Entity();\n"
		"e2.asChildOf(e1);\n"
		"local e3 = e2.parent();\n"
		"assert(e1 == e3);\n"
		"e2.destroyThis();\n"
		"assert(!e2.parent());\n"
	));
}
