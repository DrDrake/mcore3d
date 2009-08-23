#ifndef ___SCRIPT_DECLARATOR___
#define ___SCRIPT_DECLARATOR___

#include "Events.h"
#include "detail/Binding.h"
#include "detail/Constructors.h"
#include "detail/Fields.h"
#include "detail/ReturnPolicies.h"
#include "detail/ScriptObject.h"

namespace script {

namespace detail {

///
/// The very base declarator. Does nothing, but stores info
///
class Declarator
{
public:
	JKBIND_API Declarator(const ScriptObject& hostObject, HSQUIRRELVM vm);

protected:
	/*!	Responsible for creating static and member function.
		\param func Address of the function, if the function is a member function use & operator before the function
			if func == null it means creating a raw function
		\param sizeofFunc Size of the function, pass 0 for static function

		A table showing the combination of the parameters:
		func    sizeofFunc    type of function
		!null	> 0           member function
		!null   == 0          static function
		null    ignored       raw function
	 */
	JKBIND_API void pushFunction(const xchar* name, void* func, size_t sizeofFunc, SQFUNCTION dispatchFunc);

	ScriptObject _hostObject;
	HSQUIRRELVM _vm;
};	// Declarator

class ClassDeclaratorBase : public Declarator
{
protected:
	JKBIND_API ClassDeclaratorBase(const ScriptObject& hostObject, HSQUIRRELVM vm, const xchar* className);

	JKBIND_API void enableGetset();

	JKBIND_API void runScript(const xchar* script);

	const xchar* mClassName;
};	// ClassDeclaratorBase

///
/// Declares new class members in script machine
///
template<typename T>
class ClassDeclarator : public ClassDeclaratorBase
{
public:
	typedef T Class;

	ClassDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm, const xchar* className)
		: ClassDeclaratorBase(hostObject, vm, className)
	{
	}

	// Allow the user to have total control.
	// Func should have the signature as int(*)(HSQUIRRELVM)
	template<typename Func>
	ClassDeclarator& rawMethod(const xchar* name, Func func)
	{
		pushFunction(name, NULL, 0, func);
		return *this;
	}

	//
	// Constructors
	//

	ClassDeclarator& constructor(const xchar* name = xSTRING("constructor"))
	{
		T*(*fPtr)() = &constructionFunction<T>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1>
	ClassDeclarator& constructor(const xchar* name = xSTRING("constructor"))
	{
		T*(*fPtr)(A1) = &constructionFunction<T, A1>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2>
	ClassDeclarator& constructor(const xchar* name = xSTRING("constructor"))
	{
		T*(*fPtr)(A1, A2) = &constructionFunction<T, A1, A2>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2, typename A3>
	ClassDeclarator& constructor(const xchar* name = xSTRING("constructor"))
	{
		T*(*fPtr)(A1, A2, A3) = &constructionFunction<T, A1, A2, A3>;
		return staticMethod<construct>(name, fPtr);
	}

	template<typename A1, typename A2, typename A3, typename A4>
	ClassDeclarator& constructor(const xchar* name = xSTRING("constructor"))
	{
		T*(*fPtr)(A1, A2, A3, A4) = &constructionFunction<T, A1, A2, A3, A4>;
		return staticMethod<construct>(name, fPtr);
	}

	//
	// callee->method
	//

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& method(const xchar* name, Func func)
	{
		pushFunction(name, &func, sizeof(func),
			&DirectCallInstanceMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& method(const xchar* name, Func func)
	{
		pushFunction(name, &func, sizeof(func),
			&DirectCallInstanceMemberFunction<Class, Func>::Dispatch);
		return *this;
	}

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& wrappedMethod(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&IndirectCallInstanceMemberFunction<Class, Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& wrappedMethod(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&IndirectCallInstanceMemberFunction<Class, Func>::Dispatch);
		return *this;
	}

	//
	// static method
	//

	template<typename ReturnPolicy, typename Func>
	ClassDeclarator& staticMethod(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&DirectCallFunction<Func, ReturnPolicy>::Dispatch);
		return *this;
	}

	template<typename Func>
	ClassDeclarator& staticMethod(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&DirectCallFunction<Func>::Dispatch);
		return *this;
	}

	//
	// script event
	//

	template<typename Event>
	ClassDeclarator& scriptEvent(const xchar* name, Event event)
	{
		pushFunction(name, &event, sizeof(event), &eventObjectGetter<Class, Event>);
		return *this;
	}

	//
	// fields setter
	//

	template<typename Field>
	ClassDeclarator& setter(const xchar* name, Field field)
	{
		pushFunction(name, &field, sizeof(field), &fieldSetterFunction<Class, Field>);
		return *this;
	}

	// For a getter, we assume the return policy is either plain or objNoCare
	// If a custom return policy is really needed, wrappedMethod can be used instead.
	template<typename Field>
	ClassDeclarator& getter(const xchar* name, Field field)
	{
		typedef typename DetectFieldType<Field>::type fieldType;
		typedef typename DefaultReturnPolicy<fieldType>::policy returnPolicy;
		typedef typename GetterReturnPolicy<returnPolicy>::policy getterReturnPolicy;
		pushFunction(name, &field, sizeof(field),
			&fieldGetterFunction<Class, Field, getterReturnPolicy>);
		return *this;
	}

	template<typename Field>
	ClassDeclarator& getset(const xchar* name, Field field)
	{
		xchar getBuffer[64] = xSTRING("_get");
		xchar setBuffer[64] = xSTRING("_set");

		// Shut up MSVC code analysis warnings
		if(scstrlen(name) + 5 > (sizeof getBuffer/sizeof(xchar)))
			return *this;

		scscat(getBuffer, name);
		scscat(setBuffer, name);
		getter(getBuffer, field);
		setter(setBuffer, field);

		return *this;
	}

	// Transform member variable XXX access into _getXXX() and _setXXX() function
	// Behind the scene a squirrel function is injected for the class which convert
	// the filed name to a function name:
	// className._get<-function(i) {
	//   local g = ::className["_get"+i.tostring()];
	//   return g == null ? null : g();
	// }
	ClassDeclarator& enableGetset()
	{
		ClassDeclaratorBase::enableGetset();
		return *this;
	}

	ClassDeclarator& runScript(const xchar* script)
	{
		ClassDeclaratorBase::runScript(script);
		return *this;
	}
};	// ClassDeclarator

///
/// Declares new classes, global functions etc in script machine
///
class GlobalDeclarator: public Declarator
{
public:
	JKBIND_API GlobalDeclarator(const ScriptObject& hostObject, HSQUIRRELVM vm);

	JKBIND_API GlobalDeclarator declareNamespace(const xchar* name);

	template<typename T>
	ClassDeclarator<T> declareClass(const xchar* className)
	{
		return ClassDeclarator<T>(
			pushClass(className, ClassTraits<T>::classID(), typeid(T)),
			_vm, className
		);
	}

	template<typename T, typename PARENT>
	ClassDeclarator<T> declareClass(const xchar* className)
	{
		return ClassDeclarator<T>(
			pushClass(className, ClassTraits<T>::classID(), typeid(T), ClassTraits<PARENT>::classID()),
			_vm, className
		);
	}

	template<typename ReturnPolicy, typename Func>
	GlobalDeclarator declareFunction(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&DirectCallFunction<Func, ReturnPolicy>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	template<typename Func>
	GlobalDeclarator declareFunction(const xchar* name, Func func)
	{
		pushFunction(name, (void*)func, 0,
			&DirectCallFunction<Func>::Dispatch);
		return GlobalDeclarator(_hostObject, _vm);
	}

	// Allow the user to have total control.
	// Func should have the signature as int(*)(HSQUIRRELVM)
	template<typename Func>
	GlobalDeclarator rawFunction(const xchar* name, Func func)
	{
		pushFunction(name, NULL, 0, func);
		return GlobalDeclarator(_hostObject, _vm);
	}

protected:
	JKBIND_API ScriptObject pushClass(const xchar* className, ClassID classID, const std::type_info& typeID, ClassID parentClassID=0);
};	// GlobalDeclarator

}	//namespace detail
}	//namespace script

namespace script {

class RootDeclarator: public detail::GlobalDeclarator
{
public:
	JKBIND_API RootDeclarator(script::VMCore* targetVM);

private:
	JKBIND_API detail::ScriptObject _getRoot(HSQUIRRELVM vm);
};	// RootDeclarator

}   //namespace script

#endif//___SCRIPT_DECLARATOR___
