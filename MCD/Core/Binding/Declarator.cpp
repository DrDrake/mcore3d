#include "Pch.h"
#include "Declarator.h"
#include "Classes.h"
#include "VMCore.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

namespace MCD {
namespace Binding {

Declarator::Declarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
	: _hostObject(hostObject), _vm(vm)
{
}

void Declarator::pushFunction(const char* name, void* func, size_t sizeofFunc, int paramCountCheck, SQFUNCTION dispatchFunc, const ScriptObject& whereToPush)
{
	const SQInteger oldTop = sq_gettop(_vm);
	(void)oldTop;

	sq_pushobject(_vm, whereToPush.handle());
	sq_pushstring(_vm, name, -1);
	/// stack: whereToPush, name

	// Raw function
	if(!func)
		sq_newclosure(_vm, dispatchFunc, 0);
	// Static function
	else if(sizeofFunc == 0)
	{
		sq_pushuserpointer(_vm, func);
		sq_newclosure(_vm, dispatchFunc, 1);
	}
	// Member function
	else
	{	// This block of code do the same as pushFunctionPointer()
		SQUserPointer data = sq_newuserdata(_vm, sizeofFunc);
		::memcpy(data, func, sizeofFunc);
		sq_newclosure(_vm, dispatchFunc, 1);
	}
	/// stack: whereToPush, name, new closure

	CAPI_VERIFY(sq_setnativeclosurename(_vm, -1, name));

	// Count the "this" as one of the parameters
	paramCountCheck = paramCountCheck > 0 ? paramCountCheck + 1 : paramCountCheck;
	CAPI_VERIFY(sq_setparamscheck(_vm, paramCountCheck, nullptr));

	CAPI_VERIFY(sq_newslot(_vm, -3, true));
	sq_poptop(_vm);

	MCD_ASSERT(oldTop == sq_gettop(_vm));
}

void Declarator::pushFunction(const char* name, void* func, size_t sizeofFunc, int paramCountCheck, const SQFUNCTION dispatchFunc)
{
	pushFunction(name, func, sizeofFunc, paramCountCheck, dispatchFunc, _hostObject);
}

void ClassDeclaratorBase::pushVarGetSetFunction(const char* name, void* varPtr, size_t sizeofVar, SQFUNCTION dispatchGetFunc, SQFUNCTION dispatchSetFunc)
{
	pushFunction(name, varPtr, sizeofVar, 0, dispatchGetFunc, getterTable());
	pushFunction(name, varPtr, sizeofVar, 1, dispatchSetFunc, setterTable());
}

ClassDeclaratorBase::ClassDeclaratorBase(const ScriptObject& hostObject, HSQUIRRELVM vm, const char* className)
	: Declarator(hostObject, vm), mClassName(className)
{
}

ScriptObject ClassDeclaratorBase::getterTable()
{
	sq_pushobject(_vm, _hostObject.handle());
	sq_pushstring(_vm, "__getTable", -1);
	CAPI_VERIFY(sq_get(_vm, -2));

	ScriptObject ret(_vm);
	MCD_VERIFY(ret.getFromStack(-1));
	sq_pop(_vm, 2);

	return ret;
}

ScriptObject ClassDeclaratorBase::setterTable()
{
	sq_pushobject(_vm, _hostObject.handle());
	sq_pushstring(_vm, "__setTable", -1);
	CAPI_VERIFY(sq_get(_vm, -2));

	ScriptObject ret(_vm);
	MCD_VERIFY(ret.getFromStack(-1));
	sq_pop(_vm, 2);

	return ret;
}


void ClassDeclaratorBase::runScript(const char* script)
{
	MCD_VERIFY(VMCore::runScript(_vm, script));
}

GlobalDeclarator::GlobalDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
	: Declarator(hostObject, vm)
{
}

GlobalDeclarator GlobalDeclarator::declareNamespace(const char* ns)
{
	if(!_hostObject.exists(ns)) {
		sq_pushobject(_vm, _hostObject.handle());	/// stack: hostObject
		sq_pushstring(_vm, ns, -1);					/// stack: hostObject, ns
		sq_newtable(_vm);							/// stack: hostObject, ns, new table

		ScriptObject newHost(_vm);
		MCD_VERIFY(newHost.getFromStack(-1));

		CAPI_VERIFY(sq_createslot(_vm, -3));		/// stack: hostObject
		sq_poptop(_vm);

		return GlobalDeclarator(newHost, _vm);
	}

	return GlobalDeclarator(_hostObject.getValue(ns), _vm);
}

ScriptObject GlobalDeclarator::pushClass(const char* className, ClassID classID, const std::type_info& typeID, ClassID parentClassID)
{
	ClassesManager::setClassIdForRtti(typeID, classID);

	const SQInteger oldTop = sq_gettop(_vm);

	ScriptObject newClass = ClassesManager::createClass(_vm, _hostObject, classID, className, parentClassID);

	// TODO: Calling disableCloningForClass here would override the base class's _cloned function.
	// Therefore it is more preferable to make a mother of all base class to perform the disabling.
//	ClassesManager::disableCloningForClass(_vm, newClass);
	ClassesManager::createMemoryControllerSlot(_vm, newClass);
	ClassesManager::registerTypeOf(_vm, newClass, className);
	ClassesManager::registerWeakRef(_vm, newClass);
	ClassesManager::registerGetSetTable(_vm, newClass);

	MCD_ASSERT(oldTop == sq_gettop(_vm));
	sq_settop(_vm, oldTop);

	return newClass;
}

RootDeclarator::RootDeclarator(VMCore* targetVM)
	: GlobalDeclarator(_getRoot(targetVM->getVM()), targetVM->getVM())
{
}

ScriptObject RootDeclarator::_getRoot(HSQUIRRELVM vm)
{
	sq_pushroottable(vm);
	ScriptObject root(vm);
	MCD_VERIFY(root.getFromStack(-1));
	sq_poptop(vm);
	return root;
}

}	// namespace Binding
}	// namespace MCD
