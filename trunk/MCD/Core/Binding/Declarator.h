#ifndef __MCD_CORE_BINDING_DECLARATOR__
#define __MCD_CORE_BINDING_DECLARATOR__

#include "Binding.h"
#include "Constructors.h"
//#include "Events.h"
#include "Fields.h"
//#include "detail/ReturnPolicies.h"
#include "ScriptObject.h"

namespace MCD {
namespace Binding {

class VMCore;
typedef void* ClassID;

/// Detect the number of arguments of a function,
/// to serve for argument count validation.
struct FuncParamCount
{
// Static functions:
	template<class RT>
	static size_t count(RT (*func)()) { return 0; }
	template<class RT, class P1>
	static size_t count(RT (*func)(P1)) { return 1; }
	template<class RT, class P1,class P2>
	static size_t count(RT (*func)(P1,P2)) { return 2; }
	template<class RT, class P1,class P2,class P3>
	static size_t count(RT (*func)(P1,P2,P3)) { return 3; }
	template<class RT, class P1,class P2,class P3,class P4>
	static size_t count(RT (*func)(P1,P2,P3,P4)) { return 4; }
	template<class RT, class P1,class P2,class P3,class P4,class P5>
	static size_t count(RT (*func)(P1,P2,P3,P4,P5)) { return 5; }
	template<class RT, class P1,class P2,class P3,class P4,class P5,class P6>
	static size_t count(RT (*func)(P1,P2,P3,P4,P5,P6)) { return 6; }

// Member functions:
	template<class Callee, class RT>
	static size_t count(RT (Callee::*func)()) { return 0; }
	template<class Callee, class RT, class P1>
	static size_t count(RT (Callee::*func)(P1)) { return 1; }
	template<class Callee, class RT, class P1,class P2>
	static size_t count(RT (Callee::*func)(P1,P2)) { return 2; }
};	// FuncParamCount

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

	ScriptObject getterTable();
	ScriptObject setterTable();

	/*!	Modify the _get/_set meta functions to enable direct member variable get/set.
	 */
	void enableGetset();

	/*!	Register member variable reflection information
		\param explicitType Enable us to query the type even if the
			member variable is null
	 */
	void memVarReflection(const char* varName, const char* explicitType=NULL);

	void runScript(const char* script);

	/*!	Register squirrel attribute as meta data to indicate a _getXXX is a member variable.
		eg: MyClass.setattributes('_getmyVar', {varName='myVar'}
	 */
	void registerMemVarAttribute(const char* varName);

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
		pushFunction(name, &func, sizeof(func), FuncParamCount::count(func), &DirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& method(const char* name, Func func)
	{
		pushFunction(name, &func, sizeof(func), FuncParamCount::count(func), &DirectCallMemberFunction<Class, Func>::Dispatch);
		return *this;
	}

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& wrappedMethod(const char* name, Func func)
	{
		pushFunction(name, (void*)func, 0, FuncParamCount::count(func)-1, &IndirectCallMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& wrappedMethod(const char* name, Func func)
	{
		pushFunction(name, (void*)func, 0, FuncParamCount::count(func)-1, &IndirectCallMemberFunction<Class, Func>::Dispatch);
		return *this;
	}

// Static method
	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& staticMethod(const char* name, Func func)
	{
		pushFunction(name, func, 0, FuncParamCount::count(func), &DirectCallStaticFunction<Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& staticMethod(const char* name, Func func)
	{
		pushFunction(name, func, 0, FuncParamCount::count(func), &DirectCallStaticFunction<Func>::Dispatch);
		return *this;
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
	// If a custom return policy is really needed, wrappedMethod can be used instead.
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
		pushFunction(name, &field, sizeof(field), 0, &fieldGetterFunction<Class, Field, getterReturnPolicy>, getterTable());
		pushFunction(name, &field, sizeof(field), 1, &fieldSetterFunction<Class, Field>, setterTable());
		return *this;
	}

	ClassDeclarator& runScript(const char* script)
	{
		ClassDeclaratorBase::runScript(script);
		return *this;
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
		pushFunction(name, (void*)func, 0, &DirectCallStaticFunction<Func, ReturnPolicy>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	template<typename Func>
	GlobalDeclarator declareFunction(const char* name, Func func)
	{
		pushFunction(name, (void*)func, 0, &DirectCallStaticFunction<Func>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	// Allow the user to have total control.
	// Func should have the signature as SQRESULT(*)(HSQUIRRELVM)
	template<typename Func>
	GlobalDeclarator rawFunction(const char* name, Func func)
	{
		pushFunction(name, NULL, 0, FuncParamCount::count(func), func);
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
