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

void Declarator::pushFunction(const char* name, void* func, size_t sizeofFunc, int paramCountCheck, SQFUNCTION dispatchFunc)
{
	sq_pushobject(_vm, _hostObject.handle());
	sq_pushstring(_vm, name, -1);
	/// stack: _hostObject, name

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
	/// stack: _hostObject, name, new closure

	CAPI_VERIFY(sq_setnativeclosurename(_vm, -1, name));

	// Count the "this" as one of the parameters
	paramCountCheck = paramCountCheck > 0 ? paramCountCheck + 1 : paramCountCheck;
	CAPI_VERIFY(sq_setparamscheck(_vm, paramCountCheck, nullptr));

	CAPI_VERIFY(sq_newslot(_vm, -3, true));
	sq_pop(_vm, 1);
}

ClassDeclaratorBase::ClassDeclaratorBase(const ScriptObject& hostObject, HSQUIRRELVM vm, const char* className)
	: Declarator(hostObject, vm), mClassName(className)
{
}

void ClassDeclaratorBase::enableGetset()
{
	const char get1[] = "._get<-function(i){local g=::";
	const char set1[] = "._set<-function(i,v){local s=::";
	const char get2[] = "[\"_get\"+i.tostring()];return g==null?null:g()}";
	const char set2[] = "[\"_set\"+i.tostring()];return s==null?null:s(v)}";
	char buffer[256];

	// Shut up MSVC code analysis warnings
	if(::strlen(mClassName) + ((sizeof(set1) + sizeof(set2))/sizeof(char)) > (sizeof buffer/sizeof(char)))
		return;

	::strcpy(buffer, mClassName);
	::strcat(buffer, get1);
	::strcat(buffer, mClassName);
	::strcat(buffer, get2);
	runScript(buffer);
	::strcpy(buffer, mClassName);
	::strcat(buffer, set1);
	::strcat(buffer, mClassName);
	::strcat(buffer, set2);
	runScript(buffer);

	::strcpy(buffer, mClassName);
	::strcat(buffer, "._memVarsReflect <- [];");
	runScript(buffer);
}

void ClassDeclaratorBase::memVarReflection(const char* varName, const char* explicitType)
{
	char buffer[256];
	::strcpy(buffer, mClassName);
	::strcat(buffer, "._memVarsReflect.append(\"");
	::strcat(buffer, varName);
	::strcat(buffer, "\");");
	runScript(buffer);
}

void ClassDeclaratorBase::runScript(const char* script)
{
//	MCD_VERIFY(script::runScript(_vm, script));
}

void ClassDeclaratorBase::registerMemVarAttribute(const char* varName)
{
	assert(varName[0] == '_');
	assert(varName[1] == 'g');
	assert(varName[2] == 'e');
	assert(varName[3] == 't');

	std::string str = mClassName;
	str += ".setattributes(\"";
	str += varName;
	str += "\", {varName=\"";
	str += (varName + 4);	// Skip the "_get"
	str += "\"});";
	this->runScript(str.c_str());
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
		sq_pop(_vm, 1);

		return GlobalDeclarator(newHost, _vm);
	}

	return GlobalDeclarator(_hostObject.getValue(ns), _vm);
}

ScriptObject GlobalDeclarator::pushClass(const char* className, ClassID classID, const std::type_info& typeID, ClassID parentClassID)
{
	if(ClassesManager::associateClassID())
		ClassesManager::associateClassID()(typeID, classID);

	const int oldTop = sq_gettop(_vm);

	ScriptObject newClass = ClassesManager::createClass(_vm, _hostObject, classID, className, parentClassID);

	// TODO: Calling disableCloningForClass here would override the base class's _cloned function.
	// Therefore it is more preferable to make a mother of all base class to perform the disabling.
//	ClassesManager::disableCloningForClass(_vm, newClass);
	ClassesManager::createMemoryControllerSlot(_vm, newClass);
	ClassesManager::registerTypeOf(_vm, newClass, className);

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
	sq_pop(vm, 1);
	return root;
}

}	// namespace Binding

}	// namespace MCD
