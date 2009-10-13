#include "Pch.h"
#include "Entity.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

namespace types {

void addHandleToObject(HSQUIRRELVM v, Entity* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, Entity* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

void addHandleToObject(HSQUIRRELVM v, Component* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, Component* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

ClassID getClassIDFromObject(const Component* obj, ClassID original) {
	return getClassIDFromTypeInfo(typeid(*obj), original);
}

}	// namespace types

/*!	Associate a script object to the Component, such that when a C++ function
	returns a most derived Component object (even the Component is sub-classed
	in the scripting side) the actual script object is returned instead of
	a plain Component object only.
	Used in ScriptComponentManager when creating a scripted Component.
 */
static int componentSetScriptHandle(HSQUIRRELVM vm)
{
	Component* c = get(types::TypeSelect<Component*>(), vm, 1);
	c->scriptOwnershipHandle.setHandle(vm, -1);
	return 1;
}
static void componentDestroySelf(GiveUpOwnership<Component*> obj)
{
	delete obj;
}
SCRIPT_CLASS_REGISTER_NAME(Component, "Component")
	.enableGetset()
	.method<objNoCare>(xSTRING("_getentity"), &Component::entity)
	.rawMethod(xSTRING("_setScriptHandle"), &componentSetScriptHandle)
	.wrappedMethod(xSTRING("destroySelf"), &componentDestroySelf)
;}

static void entityAddChild(Entity& self, GiveUpOwnership<Entity*> e) {
	e.value->asChildOf(&self);
}
static void entityInsertBefore(Entity& self, GiveUpOwnership<Entity*> e) {
	self.insertBefore(e);
}
static void entityInsertAfter(Entity& self, GiveUpOwnership<Entity*> e) {
	self.insertAfter(e);
}
static Entity* entityUnlink(Entity& e) {
	Entity* ret = e.parent()==nullptr?nullptr:&e;
	e.unlink();
	return ret;
}
static void entityAddComponent(Entity& self, GiveUpOwnership<Component*> c) {
	self.addComponent(c);
}
SCRIPT_CLASS_REGISTER_NAME(Entity, "Entity")
	.enableGetset()
	.constructor()
	.wrappedMethod(xSTRING("addChild"), &entityAddChild)
	.wrappedMethod(xSTRING("insertBefore"), &entityInsertBefore)
	.wrappedMethod(xSTRING("insertAfter"), &entityInsertAfter)
	.wrappedMethod(xSTRING("unlink"), &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
	.wrappedMethod(xSTRING("addComponent"), &entityAddComponent)
	.getset(xSTRING("enabled"), &Entity::enabled)
	.getset(xSTRING("name"), &Entity::name)
	.getset(xSTRING("localTransform"), &Entity::localTransform)
	.method(xSTRING("_getworldTransform"), &Entity::worldTransform)
	.method(xSTRING("_setworldTransform"), &Entity::setWorldTransform)
	.method<objNoCare>(xSTRING("_getparentNode"), &Entity::parent)		// The node's life time is controled by the
	.method<objNoCare>(xSTRING("_getfirstChild"), &Entity::firstChild)	// node tree's root node, therefore we use
	.method<objNoCare>(xSTRING("_getnextSibling"), &Entity::nextSibling)// objNoCare as the return policy.
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
