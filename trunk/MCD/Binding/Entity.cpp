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

static Component* componentClone(const Component* c) {
	return c ? c->clone() : nullptr;
}
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
	.clone<&componentClone>()
	.method<objNoCare>(xSTRING("_getentity"), &Component::entity)
	.rawMethod(xSTRING("_setScriptHandle"), &componentSetScriptHandle)
	.wrappedMethod(xSTRING("destroySelf"), &componentDestroySelf)
	// The base serialize() function that responsible for construction and assigning itself to Entity.
	// Derived components should override this script function for serializing their specific data.
	.runScript(
		xSTRING("Component.serialize<-function(state) {")
		xSTRING("local name = state.getObjName(this, \"c\");")
		xSTRING("local entityName = state.getObjName(entity, null);")
		xSTRING("state.output += ::format(")
		xSTRING("@\"\tlocal %s = %s;\n")
		xSTRING("\t%s.addComponent(%s);\n")
		xSTRING("\", name, classString, entityName, name);")
		xSTRING("state.resolveReference(this);}")
	);
;}

// This clone function will not clone the Components and Entity's children,
// those objects' clonning are performed by script.
static Entity* entityClone(const Entity* e)
{
	Entity* newEnt = new Entity;
	newEnt->enabled = e->enabled;
	newEnt->name = e->name;
	newEnt->localTransform = e->localTransform;
	return newEnt;
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
static Entity* entityUnlink(Entity& e) {
	Entity* ret = e.parent()==nullptr?nullptr:&e;
	e.unlink();
	return ret;
}
static void entityAddComponent(Entity& self, GiveUpOwnership<Component*> c) {
	self.addComponent(c);
}
static Component* entityNextComponent(Entity& self, Component* c) {
	c = (c == nullptr) ? self.components.begin() : c->next();
	return c == self.components.end() ? nullptr : c;
}
SCRIPT_CLASS_REGISTER_NAME(Entity, "Entity")
	.enableGetset()
	.constructor(xSTRING("_orgConstructor"))
	.clone<&entityClone>(xSTRING("_orgCloned"))
	.wrappedMethod(xSTRING("addChild"), &entityAddChild)
	.wrappedMethod(xSTRING("insertBefore"), &entityInsertBefore)
	.wrappedMethod(xSTRING("insertAfter"), &entityInsertAfter)
	.wrappedMethod(xSTRING("unlink"), &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
	.wrappedMethod(xSTRING("addComponent"), &entityAddComponent)
	.method(xSTRING("isAncestorOf"), &Entity::isAncestorOf)
	.method<objNoCare>(xSTRING("findEntityByPath"), &Entity::findEntityByPath)
	.method(xSTRING("getRelativePathFrom"), &Entity::getRelativePathFrom)
	.getset(xSTRING("enabled"), &Entity::enabled)
	.getset(xSTRING("name"), &Entity::name)
	.getset(xSTRING("localTransform"), &Entity::localTransform)
	.method(xSTRING("_getworldTransform"), &Entity::worldTransform)
	.method(xSTRING("_setworldTransform"), &Entity::setWorldTransform)
	.method<objNoCare>(xSTRING("_getparentNode"), (Entity* (Entity::*)())(&Entity::parent))			// The node's life time is controled by the
	.method<objNoCare>(xSTRING("_getfirstChild"), (Entity* (Entity::*)())(&Entity::firstChild))		// node tree's root node, therefore we use
	.method<objNoCare>(xSTRING("_getnextSibling"), (Entity* (Entity::*)())(&Entity::nextSibling))	// objNoCare as the return policy.
	.wrappedMethod<objNoCare>(xSTRING("_nextComponent"), &entityNextComponent)
	.runScript(xSTRING("Entity._getcomponents<-function(){local c;for(;c=_nextComponent(c);)yield c;}return null;"))	// Generator for foreach
	.runScript(xSTRING("Entity.directSerialize<-null;"))
	.runScript(xSTRING("Entity.deferSerialize<-function(state){::entityDeferSerializeTraverse(this,state);}"))
	.runScript(xSTRING("Entity.serialize<-function(state){directSerialize?directSerialize(state):deferSerialize(state);}"))
	.runScript(xSTRING("Entity.constructor<-function(name=\"\"){_orgConstructor.call(this);this._setname(name);directSerialize=function(state){::entityDirectSerializeTraverse(this,state);};}"))
	.runScript(xSTRING("Entity._cloned<-function(org){_orgCloned(org);")
			   xSTRING("foreach(i,c in org.components){addComponent(clone c)};")
			   xSTRING("for(local i=org.firstChild; i; i=i.nextSibling){local e=clone i;addChild(e);};")
			   xSTRING("}"))
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
