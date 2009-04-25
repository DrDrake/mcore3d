#ifndef ___SCRIPT_DECLARATOR___
#define ___SCRIPT_DECLARATOR___

#include "detail/ScriptObject.h"
#include "detail/Binding.h"
#include "detail/ClassTraits.h"
#include "detail/Constructors.h"
#include "detail/Classes.h"
#include "detail/Fields.h"

#include "detail/ReturnPolicies.h"

namespace script {
namespace detail {

///
/// The very base declarator. Does nothing, but stores info
///
class Declarator
{
public:
	explicit Declarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
		: _hostObject(hostObject),
		_vm(vm)
	{
	}

	virtual ~Declarator()
	{
	}

protected:
	ScriptObject _hostObject;
	HSQUIRRELVM _vm;
};

///
/// Declares new class members in script machine
///
template<typename T>
class ClassDeclarator: public Declarator
{
public:
	typedef T Class;

	ClassDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
		:Declarator(hostObject, vm)
	{
	}

	//
	// Constructors
	//

	ClassDeclarator& constructor(const xchar* name) {
		return staticMethod<DefaultReturnPolicy<T>::policy>(name, &constructionFunction<T>);
	}

	template<typename A1>
	ClassDeclarator& constructor(const xchar* name) {
		return staticMethod<DefaultReturnPolicy<T>::policy>(name, &constructionFunction<T, A1>);
	}

	template<typename A1, typename A2>
	ClassDeclarator& constructor(const xchar* name) {
		return staticMethod<DefaultReturnPolicy<T>::policy>(name, &constructionFunction<T, A1, A2>);
	}

	template<typename A1, typename A2, typename A3>
	ClassDeclarator& constructor(const xchar* name) {
		return staticMethod<DefaultReturnPolicy<T>::policy>(name, &constructionFunction<T, A1, A2, A3>);
	}

	template<typename A1, typename A2, typename A3, typename A4>
	ClassDeclarator& constructor(const xchar* name) {
		return staticMethod<DefaultReturnPolicy<T>::policy>(name, &constructionFunction<T, A1, A2, A3, A4>);
	}

	//
	// callee->method
	//

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& method(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::DirectCallInstanceMemberFunction<Class, Func, ReturnPolicy>::Dispatch, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	template<typename Func>
	ClassDeclarator& method(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::DirectCallInstanceMemberFunction<Class, Func>::Dispatch, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& wrappedMethod(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::IndirectCallInstanceMemberFunction<Class, Func, ReturnPolicy>::Dispatch, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	template<typename Func>
	ClassDeclarator& wrappedMethod(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::IndirectCallInstanceMemberFunction<Class, Func>::Dispatch, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	//
	// static method
	//

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& staticMethod(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::DirectCallFunction<Func, ReturnPolicy>::Dispatch,1);
		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping host object

		return *this;
	}

	template<typename Func>
	ClassDeclarator& staticMethod(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm, script::detail::DirectCallFunction<Func>::Dispatch,1);
		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping host object

		return *this;
	}

	//
	// script event
	//

	template<typename Event>
	ClassDeclarator& scriptEvent(const xchar* name, Event event)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushEventPointer(_vm, event);
		sq_newclosure(_vm, &detail::eventObjectGetter<Class, Event>, 1);
		jkSCRIPT_API_VERIFY(sq_createslot(_vm, -3));
		sq_pop(_vm, 1); //popping host object
		return *this;
	}

	//
	// fields setter
	//

	template<typename Field>
	ClassDeclarator& setter(const xchar* name, Field field)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushFieldPointer(_vm, field);
		sq_newclosure(_vm, script::detail::fieldSetterFunction<Class, Field>, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	// For a getter, we assume the return policy is either plain or objNoCare
	// If a custom return policy is really needed, wrappedMethod can be used instead.
	template<typename Field>
	ClassDeclarator& getter(const xchar* name, Field field)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushFieldPointer(_vm, field);
		typedef typename DetectFieldType<Field>::type fieldType;
		typedef typename DefaultReturnPolicy<fieldType>::policy returnPolicy;
		typedef typename GetterReturnPolicy<returnPolicy>::policy getterReturnPolicy;
		sq_newclosure(_vm, script::detail::fieldGetterFunction<Class, Field, getterReturnPolicy>, 1);

		jkSCRIPT_API_VERIFY(sq_newslot(_vm, -3, true));
		sq_pop(_vm, 1); //popping class

		return *this;
	}

	template<typename Field>
	ClassDeclarator& getset(const xchar* name, Field field)
	{
		xchar getBuffer[64] = xSTRING("_get");
		xchar setBuffer[64] = xSTRING("_set");
		::wcscat(getBuffer, name);
		::wcscat(setBuffer, name);
		getter(getBuffer, field);
		setter(setBuffer, field);

		return *this;
	}
};

///
/// Declares new classes, global functions etc in script machine
///
class GlobalDeclarator: public Declarator
{
public:
	GlobalDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm)
		:Declarator(hostObject, vm)
	{
	}

	GlobalDeclarator declareNamespace(const xchar* name)
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

	template<typename T>
	ClassDeclarator<T> declareClass(const xchar* scriptClassName)
	{
		int top = sq_gettop(_vm);

		ScriptObject newClass = ClassesManager::createClass(_vm, _hostObject, (SQUserPointer)ClassTraits<T>::classID(), scriptClassName, 0);

		ClassesManager::disableCloningForClass(_vm, newClass);
//		ClassesManager::disableCreatingFromScriptForClass(_vm, newClass);
		ClassesManager::createMemoryControllerSlotForClass(_vm, newClass);

		sq_settop(_vm,top);
		return ClassDeclarator<T>(newClass, _vm);
	}

	template<typename T, typename PARENT>
	ClassDeclarator<T> declareClass(const xchar* scriptClassName)
	{
		int top = sq_gettop(_vm);

		ScriptObject newClass = ClassesManager::createClass(_vm, _hostObject, ClassTraits<T>::classID(), scriptClassName, ClassTraits<PARENT>::classID());

		ClassesManager::disableCloningForClass(_vm, newClass);
//		ClassesManager::disableCreatingFromScriptForClass(_vm, newClass);
		ClassesManager::createMemoryControllerSlotForClass(_vm, newClass);

		sq_settop(_vm,top);
		return ClassDeclarator<T>(newClass, _vm);
	}

	template<typename ResultPolicy, typename Func>
	GlobalDeclarator declareFunction(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm,script::detail::DirectCallFunction<Func, ResultPolicy>::Dispatch,1);
		jkSCRIPT_API_VERIFY(sq_createslot(_vm, -3));
		sq_pop(_vm, 1); //popping host object

		return GlobalDeclarator(_hostObject, _vm);
	}

	template<typename Func>
	GlobalDeclarator declareFunction(const xchar* name, Func func)
	{
		sq_pushobject(_vm, _hostObject.getObjectHandle());
		sq_pushstring(_vm, name, -1);
		pushStaticFunctionPointer(_vm, func);
		sq_newclosure(_vm,script::detail::DirectCallFunction<Func>::Dispatch,1);
		jkSCRIPT_API_VERIFY(sq_createslot(_vm, -3));
		sq_pop(_vm, 1); //popping host object

		return GlobalDeclarator(_hostObject, _vm);
	}
};

}	//namespace detail
}	//namespace script

namespace script {

class RootDeclarator: public detail::GlobalDeclarator
{
public:
	RootDeclarator(script::VMCore* targetVM)
		: GlobalDeclarator(_getRoot(targetVM->getVM()), targetVM->getVM())
	{
	}

private:
	detail::ScriptObject _getRoot(HSQUIRRELVM vm)
	{
		sq_pushroottable(vm);
		detail::StackHandler sh(vm);
		detail::ScriptObject root(vm, sh.getObjectHandle(sh.getParamCount()));
		sq_pop(vm, 1);
		return root;
	}
};

}   //namespace script

#endif//___SCRIPT_DECLARATOR___
