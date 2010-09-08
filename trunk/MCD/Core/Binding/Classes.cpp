#include "Pch.h"
#include "Classes.h"
#include "VMCore.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

namespace MCD {

namespace Binding {

// NOTE: File scope static varible is used instead of class static variable
// to favour enabling delay load of jkbind.dll
static ClassesManager::AssociateClassID gAssociateClassID = nullptr;

ClassesManager::AssociateClassID ClassesManager::associateClassID() {
	return gAssociateClassID;
}

void ClassesManager::setAssociateClassID(AssociateClassID aid) {
	gAssociateClassID = aid;
}

ScriptObject ClassesManager::findClass(HSQUIRRELVM v, ClassID classId)
{
	ScriptObject result(v);

	VMCore* vm = reinterpret_cast<VMCore*>(sq_getforeignptr(v));
	HSQOBJECT& classTable = vm->mClassesTable;

	sq_pushobject(v, classTable);
	/// stack: classTable

	// NOTE: If there is memory violation near this line, most likely
	// there are multiple instances of the class table in different dll module.
	// So please make sure the binding classes are correctly dll exported.
	sq_pushuserpointer(v, classId);
	/// stack: classTable, classId

	CAPI_VERIFY(sq_get(v, -2));	// Make sure ClassTraits<T>::bind() had invoked
	/// stack: classTable, table value

	MCD_VERIFY(result.getFromStack(-1));
	sq_pop(v, 2);

	MCD_ASSERT(result.type() == OT_CLASS);

	return result;
}

void ClassesManager::createObjectInstanceOnStack(HSQUIRRELVM v, ClassID classId, const void* objPtr)
{
	// Simply push null if the object's pointer is null
	if(objPtr == nullptr) {
		sq_pushnull(v);
		return;
	}

	const int oldTop = sq_gettop(v);
	ScriptObject classObj = findClass(v, classId);
	sq_pushobject(v, classObj.handle());
	/// stack: class

	CAPI_VERIFY(sq_createinstance(v, -1));
	/// stack: class, instance

	ScriptObject intance(v);
	MCD_VERIFY(intance.getFromStack(-1));

	// Associate the instance object with the C++ pointer
	CAPI_VERIFY(sq_setinstanceup(v, -1, const_cast<void*>(objPtr)));

	sq_settop(v, oldTop);
	sq_pushobject(v, intance.handle());
}

ScriptObject ClassesManager::createClass(HSQUIRRELVM v, ScriptObject& ns, ClassID classId, const char* className, ClassID parentClass)
{
	const int oldTop = sq_gettop(v);
	(void)oldTop;

	ScriptObject newClass(v);
	sq_pushobject(v, ns.handle());
	sq_pushstring(v, className, -1);
	/// stack: ns, classname

	if(parentClass) {
		ScriptObject parent = findClass(v, parentClass);
		sq_pushobject(v, parent.handle());
	}
	/// stack: ns, classname, [parent]

	CAPI_VERIFY(sq_newclass(v, parentClass ? 1 : 0));
	/// stack: ns, classname, class

	MCD_VERIFY(newClass.getFromStack(-1));
	CAPI_VERIFY(sq_settypetag(v, -1, classId));
	CAPI_VERIFY(sq_newslot(v, -3, false));
	/// stack: ns

	sq_pop(v, 1);
	MCD_ASSERT(oldTop == sq_gettop(v));

	// Add class to VMCore's class table
	VMCore* vm = (VMCore*)sq_getforeignptr(v);
	sq_pushobject(v, vm->mClassesTable);
	sq_pushuserpointer(v, classId);
	sq_pushobject(v, newClass.handle());
	/// stack: vm->mClassTable, classId, newClass

	CAPI_VERIFY(sq_newslot(v, -3, false));
	sq_pop(v, 1);

	MCD_ASSERT(oldTop == sq_gettop(v));

	return newClass;
}

void ClassesManager::registerTypeOf(HSQUIRRELVM v, ScriptObject& classObj, const char* typeName)
{
	struct Local {
		static SQInteger func(HSQUIRRELVM v) {
			return 1;	// Simply return the type name on the stack
		}
	};	// Local

	sq_pushobject(v, classObj.handle());	/// stack: class
	sq_pushstring(v, "_typeof", -1);		/// stack: class, functionName
	sq_pushstring(v, typeName, -1);			/// stack: class, functionName, typeName
	sq_newclosure(v, &Local::func, 1);		/// stack: class, functionName, func
	CAPI_VERIFY(sq_newslot(v, -3, true));	/// stack: class
	sq_pop(v, 1);
}

void ClassesManager::disableCloning(HSQUIRRELVM v, ScriptObject& classObj)
{
	struct Local {
		static SQInteger func(HSQUIRRELVM v) {
			return sq_throwerror(v, "Cloning for this class is disabled");
		}
	};	// Local

	sq_pushobject(v, classObj.handle());	/// stack: class
	sq_pushstring(v, "_cloned", -1);		/// stack: class, name
	sq_newclosure(v, &Local::func, 0);		/// stack: class, name, func
	CAPI_VERIFY(sq_newslot(v, -3, true));	/// stack: class
	sq_pop(v, 1);
}

void ClassesManager::createMemoryControllerSlot(HSQUIRRELVM v, ScriptObject& classObj)
{
	sq_pushobject(v, classObj.handle());
	sq_pushinteger(v, MEMORY_CONTROLLER_PARAM);
	sq_pushnull(v);
	CAPI_VERIFY(sq_createslot(v, -3));
	sq_pop(v, 1);
}

}	// namespace Binding

}	// namespace MCD
