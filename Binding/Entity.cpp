#include "Pch.h"
#include "Entity.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

static bool componentPtrIsValid(const ComponentPtr& c) { return c.get() != nullptr; }
static Entity* componentGetEntity(const ComponentPtr& c) { return c ? c->entity() : nullptr; }
SCRIPT_CLASS_REGISTER_NAME(ComponentPtr, "Component")
	.enableGetset(L"Component")
	.wrappedMethod(L"isValid", &componentPtrIsValid)
	.wrappedMethod<objNoCare>(L"_getentity", &componentGetEntity)
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
SCRIPT_CLASS_REGISTER_NAME(Entity, "Entity")
	.enableGetset(L"Entity")
	.constructor()
	.wrappedMethod(L"addChild", &entityAddChild)
	.wrappedMethod(L"insertBefore", &entityInsertBefore)
	.wrappedMethod(L"insertAfter", &entityInsertAfter)
	.wrappedMethod(L"unlink", &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
//	.wrappedMethod(L"addComponent", &entityAddComponent)		// addComponent() is prohabited
	.getset(L"enabled", &Entity::enabled)
	.getset(L"name", &Entity::name)
	.getset(L"localTransform", &Entity::localTransform)
	.method<objNoCare>(L"_getparentNode", &Entity::parent)		// The node's life time is controled by the
	.method<objNoCare>(L"_getfirstChild", &Entity::firstChild)	// node tree's root node, therefore we use
	.method<objNoCare>(L"_getnextSlibing", &Entity::nextSlibing)// objNoCare as the return policy.
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<ComponentPtr>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
