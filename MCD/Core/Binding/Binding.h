#ifndef __MCD_CORE_BINDING_BINDING__
#define __MCD_CORE_BINDING_BINDING__

#include "../ShareLib.h"
#include "ReturnPolicies.h"
#include "ReturnTypeDetector.h"
#include "../../../3Party/squirrel/squirrel.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))
#define CHECK_ARG(arg) if(!match(TypeSelect<P##arg>(), v, index+arg-1)) return sq_throwerror(v, "Incorrect function argument for C closure")
#define CHECK_THIS_PTR(p) if(!(p)) return sq_throwerror(v, "Try to call member funciton on null")

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "squirrel")
#	endif
#endif

namespace MCD {
namespace Binding {

class ClassesManager;

/// Return specializations for non-void function
template<class RT, class ReturnPolicy>
struct ReturnSpecialization
{
// Static functions:
	static int Call(RT (*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = func();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1>
	static int Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2>
	static int Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3>
	static int Call(RT (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(2);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

// Member functions:
	template<class Callee>
	static int Call(Callee & callee, RT (Callee::*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1>
	static int Call(Callee & callee, RT (Callee::*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2>
	static int Call(Callee & callee, RT (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3>
	static int Call(Callee & callee, RT (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(3);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P2>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

// Const member functions:
	template<class Callee>
	static int Call(Callee & callee, RT (Callee::*func)() const, HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1>
	static int Call(Callee & callee, RT (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2>
	static int Call(Callee & callee, RT (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3>
	static int Call(Callee & callee, RT (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(3);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P2>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}
};	// ReturnSpecialization, non-void function

/// Return specialization for void function
template<class ReturnPolicy>
struct ReturnSpecialization<void, ReturnPolicy>
{
// Static function:
	static int Call(void (*func)(), HSQUIRRELVM v, int index) {
		func();
		return 0;
	}

	template<class P1>
	static int Call(void (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class P1,class P2>
	static int Call(void (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class P1,class P2,class P3>
	static int Call(void (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(3);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
			get(TypeSelect<P2>(), v, index + 2)
		);
		return 0;
	}

// Member functions:
	template<class Callee>
	static int Call(Callee & callee, void (Callee::*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		(callee.*func)();
		return 0;
	}

	template<class Callee, class P1>
	static int Call(Callee & callee, void (Callee::*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class Callee, class P1,class P2>
	static int Call(Callee & callee, void (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3>
	static int Call(Callee & callee, void (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(3);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P2>(), v, index + 2)
		);
		return 0;
	}

// Const member functions:
	template<class Callee>
	static int Call(Callee & callee, void (Callee::*func)() const, HSQUIRRELVM v, int index) {
		(void)index;
		(callee.*func)();
		return 0;
	}

	template<class Callee, class P1>
	static int Call(Callee & callee, void (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class Callee, class P1,class P2>
	static int Call(Callee & callee, void (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3>
	static int Call(Callee & callee, void (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		CHECK_ARG(3);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P2>(), v, index + 2)
		);
		return 0;
	}
};	// ReturnSpecialization, void function

// Call(), aims to decompose the incomming function's reutrn and parameter types

// Static function callers
template<class ResultPolicy, class RT>
int Call(RT (*func)(), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1>
int Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2>
int Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3>
int Call(RT (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

// Member function callers
template<class ResultPolicy, class Callee, class RT>
int Call(Callee& callee, RT (Callee::*func)(), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1>
int Call(Callee& callee, RT (Callee::*func)(P1), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2>
int Call(Callee& callee, RT (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3>
int Call(Callee& callee, RT (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

// Const member function callers
template<class ResultPolicy, class Callee, class RT>
int Call(Callee& callee, RT (Callee::*func)() const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1>
int Call(Callee& callee, RT (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2>
int Call(Callee& callee, RT (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3>
int Call(Callee& callee, RT (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

/// Direct call static function handler (static function is always direct anyway)
template<class Func, class ResultPolicy>
class DirectCallStaticFunction
{
public:
	static SQInteger Dispatch(HSQUIRRELVM v)
	{
		void* p = nullptr;
		CAPI_VERIFY(sq_getuserpointer(v, -1, &p));
		Func func = Func(p);
		MCD_ASSUME(func);
		return Call<ResultPolicy>(*func, v, 2);
	}
};

/// Direct all member function handler
template<class Callee, class Func, class ResultPolicy>
class DirectCallMemberFunction
{
public:
	static SQInteger Dispatch(HSQUIRRELVM v)
	{
		Callee* instance(nullptr);
		if(SQ_FAILED(fromInstanceUp(v, 1, instance, instance, ClassTraits<Callee>::classID())))
			return sq_throwerror(v, "Trying to invoke an member function without a correct this pointer");
		CHECK_THIS_PTR(instance);
		Func func = getFunctionPointer<Func>(v, -1);
		MCD_ASSUME(func);
		return Call<ResultPolicy, Callee>(*instance, func, v, 2);
	}
};

/// Wrapped member function handler, a static function with the first parameter
/// as the "this" pointer will be invoked
template<class Callee, class Func, class ResultPolicy>
class IndirectCallMemberFunction
{
public:
	static SQInteger Dispatch(HSQUIRRELVM v)
	{
		void* p = nullptr;
		CAPI_VERIFY(sq_getuserpointer(v, -1, &p));
		Func func = Func(p);
		MCD_ASSUME(func);
		return Call<ResultPolicy>(*func, v, 1);	// Note the index is 1 instead of 2
	}
};

}	// namespace Binding
}	// namespace MCD

#undef CAPI_VERIFY
#undef CHECK_ARG
#undef CHECK_THIS_PTR

#endif	// __MCD_CORE_BINDING_BINDING__
