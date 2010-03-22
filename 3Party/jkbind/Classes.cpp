#include "detail/Classes.h"
#include "VMCore.h"

namespace script {
namespace detail {

//
// closures
//

static SQInteger _cloneDisabler(HSQUIRRELVM v)
{
	jkSCRIPT_ERROR_CLONE_DISABLED;
	return sq_throwerror(v, xSTRING("Cloning for this class is disabled"));
}

static SQInteger _typeOf(HSQUIRRELVM v)
{
	return 1;	// Simply return the type name on the stack
}

// NOTE: File scope static varible is used instead of class static variable
// to favour enabling delay load of jkbind.dll
static ClassesManager::AssociateClassID gAssociateClassID = NULL;

ClassesManager::AssociateClassID ClassesManager::associateClassID() {
	return gAssociateClassID;
}

void ClassesManager::setAssociateClassID(AssociateClassID aid) {
	gAssociateClassID = aid;
}

ScriptObject ClassesManager::_findClass(HSQUIRRELVM v, ClassID classType)
{
	ScriptObject result(v);

	VMCore* vm = reinterpret_cast<VMCore*>(sq_getforeignptr(v));
	HSQOBJECT& types = vm->_classesTable;

	sq_pushobject(v, types);

	// NOTE: If there is memory violation near this line, most likely
	// there are multiple instances of the class table in different dll module.
	// So please make sure the binding classes are correctly dll exported.
	sq_pushuserpointer(v, classType);
	jkSCRIPT_API_VERIFY(sq_get(v, -2));

	result.attachToStackObject(-1);
	sq_pop(v, 2);

	return result;
}

void ClassesManager::createObjectInstanceOnStackPure(HSQUIRRELVM v, ClassID classType, const void* c_this)
{
	// Simply push null if the object's pointer is null
	if(c_this == NULL) {
		sq_pushnull(v);
		return;
	}

	int top = sq_gettop(v);
	ScriptObject classObj = _findClass(v, classType);
	sq_pushobject(v, classObj.handle());

	//stack: types, class
	//..creating instance
	jkSCRIPT_API_VERIFY(sq_createinstance(v, -1));

	ScriptObject intance(v);
	intance.attachToStackObject(-1);

	//setting up memory controller
	jkSCRIPT_API_VERIFY(sq_setinstanceup(v, -1, const_cast<void*>(c_this)));

	sq_settop(v, top);
	sq_pushobject(v, intance.getObjectHandle());
}

ScriptObject ClassesManager::createClass(HSQUIRRELVM v, ScriptObject& root, ClassID classType, const xchar* name, ClassID parentClass)
{
	ScriptObject newClass(v);
	int oldtop = sq_gettop(v);
	sq_pushobject(v, root.getObjectHandle());	//root
	sq_pushstring(v, name, -1);					//root, classname

	if(parentClass) {
		ScriptObject parent = _findClass(v, parentClass);
		sq_pushobject(v, parent.handle());		//root, classname, [parent]
	}

	jkSCRIPT_API_VERIFY(sq_newclass(v, parentClass ? 1 : 0));	//root, classname, class

	newClass.attachToStackObject(-1);
	sq_settypetag(v, -1, classType);
	sq_newslot(v, -3, false);					//root, classname, class
	sq_pop(v, 1);								//root

	//
	// adding type to types table
	//
	VMCore* vm = (VMCore*)sq_getforeignptr(v);
	HSQOBJECT& types = vm->_classesTable;
	sq_pushobject(v, types);
	sq_pushuserpointer(v, classType);
	sq_pushobject(v, newClass.getObjectHandle());
	jkSCRIPT_API_VERIFY(sq_newslot(v, -3, false));
	sq_pop(v, 1);

	jkSCRIPT_LOGIC_ASSERT(oldtop == sq_gettop(v));
	return newClass;
}

void ClassesManager::typeofForClass(HSQUIRRELVM v, ScriptObject& classObj, const xchar* typeName)
{
	sq_pushobject(v, classObj.handle());			//class
	sq_pushstring(v, xSTRING("_typeof"), -1);		//class, functionName
	sq_pushstring(v, typeName, -1);					//class, functionName, typeName
	sq_newclosure(v, &_typeOf, 1);					//class, functionName, func
	jkSCRIPT_API_VERIFY(sq_newslot(v, -3, true));	//class
	sq_pop(v, 1);
}

void ClassesManager::disableCloningForClass(HSQUIRRELVM v, ScriptObject& classObj)
{
	sq_pushobject(v, classObj.handle());			//class
	sq_pushstring(v, xSTRING("_cloned"), -1);		//class, name
	sq_newclosure(v, &_cloneDisabler, 0);			//class, name, func
	jkSCRIPT_API_VERIFY(sq_newslot(v, -3, true));	//class
	sq_pop(v, 1);
}

void ClassesManager::createMemoryControllerSlotForClass(HSQUIRRELVM v, ScriptObject& classObj)
{
	sq_pushobject(v, classObj.handle());
	sq_pushinteger(v, MEMORY_CONTROLLER_PARAM);
	sq_pushnull(v);
	jkSCRIPT_API_VERIFY(sq_createslot(v, -3));
	sq_pop(v, 1); //popping class
}

//
// misc
//

ScriptObject createTable(HSQUIRRELVM v)
{
	ScriptObject ret(v);
	sq_newtable(v);
	ret.attachToStackObject(-1);
	sq_pop(v, 1);
	return ret;
}

ScriptObject createArray(HSQUIRRELVM v, int size)
{
	ScriptObject ret(v);
	sq_newarray(v,size);
	ret.attachToStackObject(-1);
	sq_pop(v, 1);
	return ret;
}

}	//namespace detail
}	//namespace script

//eof
//
