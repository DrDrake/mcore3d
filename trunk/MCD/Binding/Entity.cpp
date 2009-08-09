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

SCRIPT_CLASS_REGISTER_NAME(Component, "Component")
	.enableGetset(L"Component")
	.method<objNoCare>(L"_getentity", &Component::entity)
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
	.enableGetset(L"Entity")
	.constructor()
	.wrappedMethod(L"addChild", &entityAddChild)
	.wrappedMethod(L"insertBefore", &entityInsertBefore)
	.wrappedMethod(L"insertAfter", &entityInsertAfter)
	.wrappedMethod(L"unlink", &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
	.wrappedMethod(L"addComponent", &entityAddComponent)
	.getset(L"enabled", &Entity::enabled)
	.getset(L"name", &Entity::name)
	.getset(L"localTransform", &Entity::localTransform)
	.method(L"_getworldTransform", &Entity::worldTransform)
	.method(L"_setworldTransform", &Entity::setWorldTransform)
	.method<objNoCare>(L"_getparentNode", &Entity::parent)		// The node's life time is controled by the
	.method<objNoCare>(L"_getfirstChild", &Entity::firstChild)	// node tree's root node, therefore we use
	.method<objNoCare>(L"_getnextSibling", &Entity::nextSibling)// objNoCare as the return policy.
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
