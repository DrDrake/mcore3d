#include "Declarator.h"

namespace script {

namespace detail {

Declarator::Declarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
	: _hostObject(hostObject), _vm(vm)
{
}

void Declarator::pushFunction(const xchar* name, void* func, size_t sizeofFunc, SQFUNCTION dispatchFunc)
{
	sq_pushobject(_vm, _hostObject.getObjectHandle());
	sq_pushstring(_vm, name, -1);

	// Raw function
	if(func == NULL)
	{
		sq_newclosure(_vm, dispatchFunc, 0);
	}
	// Static function
	else if(sizeofFunc == 0)
	{
		sq_pushuserpointer(_vm, func);
		sq_newclosure(_vm, dispatchFunc, 1);
	}
	// Member function
	else
	{	// This block of codd do the same as pushFunctionPointer()
		SQUserPointer data = sq_newuserdata(_vm, sizeofFunc);
		::memcpy(data, func, sizeofFunc);
		sq_newclosure(_vm, dispatchFunc, 1);
	}

	jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
	sq_pop(_vm, 1); // Popping class
}

ClassDeclaratorBase::ClassDeclaratorBase(const ScriptObject& hostObject, HSQUIRRELVM vm, const xchar* className)
	: Declarator(hostObject, vm), mClassName(className)
{
}

void ClassDeclaratorBase::enableGetset()
{
	const xchar get1[] = xSTRING("._get<-function(i){local g=::");
	const xchar set1[] = xSTRING("._set<-function(i,v){local s=::");
	const xchar get2[] = xSTRING("[\"_get\"+i.tostring()];return g==null?null:g()}");
	const xchar set2[] = xSTRING("[\"_set\"+i.tostring()];return s==null?null:s(v)}");
	xchar buffer[256];

	// Shut up MSVC code analysis warnings
	if(::wcslen(mClassName) + ((sizeof(set1) + sizeof(set2))/sizeof(xchar)) > (sizeof buffer/sizeof(xchar)))
		return;

	::wcscpy(buffer, mClassName);
	::wcscat(buffer, get1);
	::wcscat(buffer, mClassName);
	::wcscat(buffer, get2);
	runScript(buffer);
	::wcscpy(buffer, mClassName);
	::wcscat(buffer, set1);
	::wcscat(buffer, mClassName);
	::wcscat(buffer, set2);
	runScript(buffer);
}

void ClassDeclaratorBase::runScript(const xchar* script)
{
	jkSCRIPT_LOGIC_VERIFY(script::runScript(_vm, script));
}

GlobalDeclarator::GlobalDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
	: Declarator(hostObject, vm)
{
}

GlobalDeclarator GlobalDeclarator::declareNamespace(const xchar* name)
{
	if(!_hostObject.exists(name)) {
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		sq_newtable(_vm);
		StackHandler sh(_vm);
		ScriptObject newHost(_vm, sh.getObjectHandle(-1));
		jkSCRIPT_API_VERIFY(sq_createslot(_vm, -3));
		sq_pop(_vm, 1);

		return GlobalDeclarator(newHost, _vm);
	}

	return GlobalDeclarator(_hostObject.getValue(name), _vm);
}

ScriptObject GlobalDeclarator::pushClass(const xchar* className, ClassID classID, const std::type_info& typeID, ClassID parentClassID)
{
	if(ClassesManager::associateClassID())
		ClassesManager::associateClassID()(typeID, classID);

	int top = sq_gettop(_vm);

	ScriptObject newClass = ClassesManager::createClass(_vm, _hostObject, classID, className, parentClassID);

	// TODO: Calling disableCloningForClass here would override the base class's _cloned function.
	// Therefore it is more preferable to make a mother of all base class to perform the disabling.
//	ClassesManager::disableCloningForClass(_vm, newClass);
	ClassesManager::createMemoryControllerSlotForClass(_vm, newClass);

	sq_settop(_vm,top);

	return newClass;
}

}	// namespace detail

RootDeclarator::RootDeclarator(script::VMCore* targetVM)
	: GlobalDeclarator(_getRoot(targetVM->getVM()), targetVM->getVM())
{
}

detail::ScriptObject RootDeclarator::_getRoot(HSQUIRRELVM vm)
{
	sq_pushroottable(vm);
	detail::StackHandler sh(vm);
	detail::ScriptObject root(vm, sh.getObjectHandle(sh.getParamCount()));
	sq_pop(vm, 1);
	return root;
}

}	// namespace script
