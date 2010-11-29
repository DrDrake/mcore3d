#ifndef __MCD_CORE_BINDING_DECLARATOR__
#define __MCD_CORE_BINDING_DECLARATOR__

#include "Binding.h"
#include "Constructors.h"
//#include "Events.h"
#include "Fields.h"
#include "ScriptObject.h"

namespace MCD {
namespace Binding {

class VMCore;
typedef void* ClassID;

/// The very base declarator. Does nothing, but stores info
class MCD_CORE_API Declarator
{
public:
	Declarator(const ScriptObject& hostObject, HSQUIRRELVM vm);

protected:
	/*!	Responsible for creating static and member function.
		\param func Address of the function, if the function is a member function
			if func == null it means creating a raw function (A function that take HSQUIRRELVM and return SQRESULT)
		\param sizeofFunc Size of the function, pass 0 for static function
		\param paramCountCheck As the number passed to sq_setparamscheck()

		A table showing the combination of the parameters:
		Func    Size Of Func  Type of function
		!null   > 0           member function
		!null   == 0          static function
		null    ignored       raw function
	 */
	void pushFunction(const char* name, void* func, size_t sizeofFunc, int paramCountCheck, SQFUNCTION dispatchFunc, const ScriptObject& whereToPush);

	void pushFunction(const char* name, void* func, size_t sizeofFunc, int paramCountCheck, SQFUNCTION dispatchFunc);

	ScriptObject _hostObject;
	HSQUIRRELVM _vm;
};	// Declarator

class MCD_CORE_API ClassDeclaratorBase : public Declarator
{
protected:
	ClassDeclaratorBase(const ScriptObject& hostObject, HSQUIRRELVM vm, const char* className);

	void runScript(const char* script);

protected:
	// Specialized pushFunction() to minimize code size
	void pushVarGetSetFunction(const char* name, void* varPtr, size_t sizeofVar, SQFUNCTION dispatchGetFunc, SQFUNCTION dispatchSetFunc);

	ScriptObject getterTable();
	ScriptObject setterTable();

	const char* mClassName;
};	// ClassDeclaratorBase

/// Declares new class members in script machine
template<typename T>
class ClassDeclarator : public ClassDeclaratorBase
{
public:
	typedef T Class;

	ClassDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm, const char* className)
		: ClassDeclaratorBase(hostObject, vm, className)
	{
	}

	// Allow the user to have total control.
	// Func should have the signature as SQRESULT(*)(HSQUIRRELVM)
	template<typename Func>
	ClassDeclarator& rawMethod(const char* name, Func func)
	{
		pushFunction(name, NULL, 0, 0, func);
		return *this;
	}

// Constructors:
	ClassDeclarator& constructor(const char* name = "constructor")
	{
		T*(*fPtr)() = &constructionFunction<T>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1>
	ClassDeclarator& constructor(const char* name = "constructor")
	{
		T*(*fPtr)(A1) = &constructionFunction<T, A1>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2>
	ClassDeclarator& constructor(const char* name = "constructor")
	{
		T*(*fPtr)(A1, A2) = &constructionFunction<T, A1, A2>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2, typename A3>
	ClassDeclarator& constructor(const char* name = "constructor")
	{
		T*(*fPtr)(A1, A2, A3) = &constructionFunction<T, A1, A2, A3>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2, typename A3, typename A4>
	ClassDeclarator& constructor(const char* name = "constructor")
	{
		T*(*fPtr)(A1, A2, A3, A4) = &constructionFunction<T, A1, A2, A3, A4>;
		return staticMethod<construct>(name, fPtr);
	}

// Clone function:
	template<T* F(const T*)>
	ClassDeclarator& clone(const char* name = "_cloned")
	{
		T*(*fPtr)(T*) = &cloneFunction<T, F>;
		return staticMethod<construct>(name, fPtr);
	}

// Member method
	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& method(const char* name, Func func)
	{
		methodDispatch<ReturnPolicy>(name, func, typename FuncTraits<Func>::FuncType());
		return *this;
	}

	template<typename Func>
	ClassDeclarator& method(const char* name, Func func)
	{
		return method<DefaultReturnPolicy<typename FuncTraits<Func>::RET>::policy>(name, func);
	}

// Static method
	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& staticMethod(const char* name, Func func)
	{
		staticMethodDispatch<ReturnPolicy>(name, func, typename FuncTraits<Func>::FuncType());
		return *this;
	}

	template<typename Func>
	ClassDeclarator& staticMethod(const char* name, Func func)
	{
		return staticMethod<DefaultReturnPolicy<typename FuncTraits<Func>::RET>::policy>(name, func);
	}

// Script event:
	template<typename Event>
	ClassDeclarator& scriptEvent(const char* name, Event event)
	{
		pushFunction(name, &event, sizeof(event), &eventObjectGetter<Class, Event>);
		return *this;
	}

// Fields get set
	// For a getter, we assume the return policy is either plain or objNoCare
	// If a custom return policy is really needed, wrap the member field using a static function.
	template<typename Field>
	ClassDeclarator& getter(const char* name, Field field)
	{
		typedef typename DetectFieldType<Field>::type fieldType;
		typedef typename DefaultReturnPolicy<fieldType>::policy returnPolicy;
		typedef typename GetterReturnPolicy<returnPolicy>::policy getterReturnPolicy;
		pushFunction(name, &field, sizeof(field), 0, &fieldGetterFunction<Class, Field, getterReturnPolicy>);
		return *this;
	}

	template<typename Field>
	ClassDeclarator& setter(const char* name, Field field)
	{
		pushFunction(name, &field, sizeof(field), 1, &fieldSetterFunction<Class, Field>);
		return *this;
	}

	template<typename Field>
	ClassDeclarator& getterSetter(const char* getterName, const char* setterName, Field field)
	{
		getter(getterName, field);
		setter(setterName, field);
		return *this;
	}

	template<typename Field>
	ClassDeclarator& var(const char* name, Field field)
	{
		typedef typename DetectFieldType<Field>::type fieldType;
		typedef typename DefaultReturnPolicy<fieldType>::policy returnPolicy;
		typedef typename GetterReturnPolicy<returnPolicy>::policy getterReturnPolicy;
		pushVarGetSetFunction(name, &field, sizeof(field),
			&fieldGetterFunction<Class, Field, getterReturnPolicy>,
			&fieldSetterFunction<Class, Field>
		);
		return *this;
	}

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& varGet(const char* name, Func func)
	{
		varGetDispatch<ReturnPolicy>(name, func, typename FuncTraits<Func>::FuncType());
		return *this;
	}

	template<typename Func>
	ClassDeclarator& varGet(const char* name, Func func)
	{
		varGetDispatch<DefaultReturnPolicy<typename FuncTraits<Func>::RET>::policy>(name, func, typename FuncTraits<Func>::FuncType());
		return *this;
	}

	template<typename Func>
	ClassDeclarator& varSet(const char* name, Func func)
	{
		varSetDispatch<plain>(name, func, typename FuncTraits<Func>::FuncType());
		return *this;
	}

	ClassDeclarator& runScript(const char* script)
	{
		ClassDeclaratorBase::runScript(script);
		return *this;
	}

protected:
// Script class member function
	template<typename ReturnPolicy, typename Field>
	void methodDispatch(const char* name, Field field, MemberField)
	{
		pushFunction(name, &field, sizeof(field), 0, &fieldGetterFunction<Class, Field, ReturnPolicy>);
	}

	template<typename ReturnPolicy, typename Func>
	void methodDispatch(const char* name, Func func, MemberFunc)
	{
		pushFunction(name, &func, sizeof(func), FuncTraits<Func>::ParamCount, &DirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
	}

	template<typename ReturnPolicy, typename Func>
	void methodDispatch(const char* name, Func func, StaticFunc)
	{
		wrappedMethod<ReturnPolicy>(name, func);
	}

	template<typename ReturnPolicy, typename Func>
	void methodDispatch(const char* name, Func func, RawSqFunc)
	{
		rawMethod(name, func);
	}

// Script class static function
	template<typename ReturnPolicy, typename Func>
	void staticMethodDispatch(const char* name, Func func, StaticFunc)
	{
		pushFunction(name, func, 0, FuncTraits<Func>::ParamCount, &DirectCallStaticFunction<Func, ReturnPolicy>::Dispatch);
	}

	template<typename ReturnPolicy, typename Func>
	void staticMethodDispatch(const char* name, Func func, RawSqFunc)
	{
		rawMethod(name, func);
	}

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& wrappedMethod(const char* name, Func func)
	{
		pushFunction(name, (void*)func, 0, FuncTraits<Func>::ParamCount-1, &IndirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
		return *this;
	}

// Script class variable get
	template<typename ReturnPolicy, typename Func>
	void varGetDispatch(const char* name, Func func, MemberFunc)
	{
		pushFunction(name, &func, sizeof(func), 0, &DirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch, getterTable());
	}

	template<typename ReturnPolicy, typename Func>
	void varGetDispatch(const char* name, Func func, StaticFunc)
	{
		pushFunction(name, (void*)func, 0, 0, &IndirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch, getterTable());
	}

// Script class variable set
	template<typename ReturnPolicy, typename Func>
	void varSetDispatch(const char* name, Func func, MemberFunc)
	{
		pushFunction(name, &func, sizeof(func), 0, &DirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch, setterTable());
	}

	template<typename ReturnPolicy, typename Func>
	void varSetDispatch(const char* name, Func func, StaticFunc)
	{
		pushFunction(name, (void*)func, 0, 0, &IndirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch, setterTable());
	}
};	// ClassDeclarator

/// Declares new classes, global functions etc in script machine
class MCD_CORE_API GlobalDeclarator: public Declarator
{
public:
	GlobalDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm);

	GlobalDeclarator declareNamespace(const char* name);

	template<typename T>
	ClassDeclarator<T> declareClass(const char* className)
	{
		return ClassDeclarator<T>(
			pushClass(className, ClassTraits<T>::classID(), typeid(T)),
			_vm, className
		);
	}

	template<typename T, typename PARENT>
	ClassDeclarator<T> declareClass(const char* className)
	{
		return ClassDeclarator<T>(
			pushClass(className, ClassTraits<T>::classID(), typeid(T), ClassTraits<PARENT>::classID()),
			_vm, className
		);
	}

	template<typename ReturnPolicy, typename Func>
	GlobalDeclarator declareFunction(const char* name, Func func)
	{
		pushFunction(name, (void*)func, 0, FuncTraits<Func>::ParamCount, &DirectCallStaticFunction<Func, ReturnPolicy>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	template<typename Func>
	GlobalDeclarator declareFunction(const char* name, Func func)
	{
		typedef typename DefaultReturnPolicy<Func>::policy returnPolicy;
		pushFunction(name, (void*)func, 0, FuncTraits<Func>::ParamCount, &DirectCallStaticFunction<Func, returnPolicy>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	// Allow the user to have total control.
	// Func should have the signature as SQRESULT(*)(HSQUIRRELVM)
	template<typename Func>
	GlobalDeclarator rawFunction(const char* name, Func func)
	{
		pushFunction(name, NULL, 0, FuncTraits<Func>::ParamCount, func);
		return GlobalDeclarator(_hostObject, _vm);
	}

protected:
	ScriptObject pushClass(const char* className, ClassID classID, const std::type_info& typeID, ClassID parentClassID=0);
};	// GlobalDeclarator

class MCD_CORE_API RootDeclarator: public GlobalDeclarator
{
public:
	RootDeclarator(VMCore* targetVM);

private:
	ScriptObject _getRoot(HSQUIRRELVM vm);
};	// RootDeclarator

}	// namespace Binding
}   // namespace MCD

#endif	// __MCD_CORE_BINDING_DECLARATOR__
