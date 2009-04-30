#include "Pch.h"
#include "Entity.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER_NAME(Component, "Component")
	.method(L"entity", &Component::entity)
;}

static bool componentPtrIsValid(const ComponentPtr& c) { return c.get() != nullptr; }

SCRIPT_CLASS_REGISTER_NAME(ComponentPtr, "ComponentPtr")
	.constructor<Component*>(L"create")
	.wrappedMethod(L"isValid", &componentPtrIsValid)
	.method<objNoCare>(L"getPointee", &ComponentPtr::get)
	.runScript(L"\
		ComponentPtr._set<-function(idx,val) { return isValid()?getPointee()[idx]=val:null; }\
		ComponentPtr._get<-function(idx) { return isValid()?getPointee()[idx]:null; }")
;}

static Entity* entityUnlink(Entity& e) {
	Entity* ret = e.parent()==nullptr?nullptr:&e;
	e.unlink();
	return ret;
}
static void entityAddChild(Entity& self, GiveUpOwnership<Entity*> e) {
	e.value->asChildOf(&self);
}
static void entityInsertBefore(Entity& self, GiveUpOwnership<Entity*> e) {
	self.insertBefore(e);
}
static void entityInsertAfter(Entity& self, GiveUpOwnership<Entity*> e) {
	self.insertAfter(e);
}
static void entityAddComponent(Entity& self, GiveUpOwnership<Component*> c) {
	self.addComponent(c);
}
SCRIPT_CLASS_REGISTER_NAME(Entity, "Entity")
	.enableGetset(L"Entity")
	.constructor(L"create")
	.wrappedMethod(L"addChild", &entityAddChild)
	.wrappedMethod(L"insertBefore", &entityInsertBefore)
	.wrappedMethod(L"insertAfter", &entityInsertAfter)
	.wrappedMethod(L"unlink", &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
	.wrappedMethod(L"__addComponent", &entityAddComponent)
	.getset(L"enabled", &Entity::enabled)
	.getset(L"name", &Entity::name)
	.getset(L"localTransform", &Entity::localTransform)
	.method<objNoCare>(L"_getparent", &Entity::parent)			// The node's life time is controled by the
	.method<objNoCare>(L"_getfirstChild", &Entity::firstChild)	// node tree's root node, therefore we use
	.method<objNoCare>(L"_getnextSlibing", &Entity::nextSlibing)// objNoCare as the return policy.
	.runScript(L"\
		// A component should not live outside an Entity \n\
		Entity.addComponent <- function(ComponentName, ...) { \
			local rt = ::getroottable(); \
			local c = rt[ComponentName].create(); \
			this.__addComponent(c); \
			return rt[ComponentName + \"Ptr\"].create(c); \
		}")
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<ComponentPtr>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
