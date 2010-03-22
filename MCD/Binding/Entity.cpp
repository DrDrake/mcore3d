#include "Pch.h"
#include "Entity.h"
#include "Binding.h"
#include "Math.h"

using namespace MCD;

namespace script {

namespace types {

void destroy(MCD::Component* obj) {
	obj->destroyThis();
}

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
	obj.value->destroyThis();
}
SCRIPT_CLASS_REGISTER_NAME(Component, "Component")
	.enableGetset()
	// There is problem for derived class to use base class's _cloned()
	// therefore an alternative name is used.
	.clone<&componentClone>("_orgCloned")
	.method<objNoCare>("_getentity", &Component::entity)
	.rawMethod("_setScriptHandle", &componentSetScriptHandle)
	.wrappedMethod("destroySelf", &componentDestroySelf)
	// The base serialize() function that responsible for construction and assigning itself to Entity.
	// Derived components should override this script function for serializing their specific data.
	.runScript(
		"Component.serialize<-function(state) {"
		"local name = state.getObjName(this, \"c\");"
		"local entityName = state.getObjName(entity, null);"
		"state.output += ::format("
		"@\"\tlocal %s = %s;\n"
		"\t%s.addComponent(%s);\n"
		"\", name, classString, entityName, name);"
		"state.resolveReference(this);}"
	)
	.runScript(
		"Component._cloned<-function(org){_orgCloned(org);}"
	)
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
	.constructor("_orgConstructor")
	.clone<&entityClone>("_orgCloned")
	.wrappedMethod("addChild", &entityAddChild)
	.wrappedMethod("insertBefore", &entityInsertBefore)
	.wrappedMethod("insertAfter", &entityInsertAfter)
	.wrappedMethod("unlink", &entityUnlink)					// If the node is unlinked, it's ownership will give to the script
	.wrappedMethod("addComponent", &entityAddComponent)
	.method("isAncestorOf", &Entity::isAncestorOf)
	.method<objNoCare>("findEntityByPath", &Entity::findEntityByPath)
	.method("getRelativePathFrom", &Entity::getRelativePathFrom)
	.getset("enabled", &Entity::enabled)
	.getset("name", &Entity::name)
	.getset("localTransform", &Entity::localTransform)
	.method("_getworldTransform", &Entity::worldTransform)
	.method("_setworldTransform", &Entity::setWorldTransform)
	.method<objNoCare>("_getparentNode", (Entity* (Entity::*)())(&Entity::parent))			// The node's life time is controled by the
	.method<objNoCare>("_getfirstChild", (Entity* (Entity::*)())(&Entity::firstChild))		// node tree's root node, therefore we use
	.method<objNoCare>("_getnextSibling", (Entity* (Entity::*)())(&Entity::nextSibling))	// objNoCare as the return policy.
	.wrappedMethod<objNoCare>("_nextComponent", &entityNextComponent)
	.runScript("Entity._getcomponents<-function(){local c;for(;c=_nextComponent(c);)yield c;}return null;")	// Generator for foreach
	.runScript("Entity.directSerialize<-null;")
	.runScript("Entity.deferSerialize<-function(state){::entityDeferSerializeTraverse(this,state);}")
	.runScript("Entity.serialize<-function(state){directSerialize?directSerialize(state):deferSerialize(state);}")
	.runScript("Entity.constructor<-function(name=\"\"){_orgConstructor.call(this);this._setname(name);directSerialize=function(state){::entityDirectSerializeTraverse(this,state);};}")
	.runScript("Entity._cloned<-function(org){_orgCloned(org);"
			   "foreach(i,c in org.components){addComponent(clone c)};"
			   "for(local i=org.firstChild, last=null; i; i=i.nextSibling){local e=clone i;if(last)e.insertAfter(last);else addChild(e);last=e;};"
			   "}")
;}

}	// namespace script

namespace MCD {

void registerEntityBinding(script::VMCore* v)
{
	script::ClassTraits<Component>::bind(v);
	script::ClassTraits<Entity>::bind(v);
}

}	// namespace MCD
