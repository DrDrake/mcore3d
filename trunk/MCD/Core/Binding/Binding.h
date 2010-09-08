#ifndef __MCD_CORE_BINDING_BINDING__
#define __MCD_CORE_BINDING_BINDING__

#include "../ShareLib.h"
#include "ReturnPolicies.h"
#include "ReturnTypeDetector.h"
#include "../../../3Party/squirrel/squirrel.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))
#define CHECK_ARG(arg) if(!match(TypeSelect<P##arg>(), v, index+arg-1)) return sq_throwerror(v, "Incorrect function argument for C closure")

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
template<class RT, typename ReturnPolicy>
struct ReturnSpecialization
{
// Static functions:
	static int Call(RT (*func)(), HSQUIRRELVM v, int index) {
		RT ret = func();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<typename P1>
	static int Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<typename P1,typename P2>
	static int Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}
};	// ReturnSpecialization, non-void function

/// Return specialization for void function
template<typename ReturnPolicy>
struct ReturnSpecialization<void, ReturnPolicy>
{
// Static function:
	static int Call(void (*func)(), HSQUIRRELVM v, int index) {
		func();
		return 0;
	}

	template<typename P1>
	static int Call(void (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<typename P1,typename P2>
	static int Call(void (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		CHECK_ARG(2);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}
};	// ReturnSpecialization, void function

// Call(), aims to decompose the incomming function's reutrn and parameter types

// Static function callers

template<typename ResultPolicy, typename RT>
int Call(RT (*func)(), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<typename ResultPolicy, typename RT,typename P1>
int Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
//	GiveUpSpecialization<P1>::giveUp(v,index+0);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2>
int Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
//	GiveUpSpecialization<P1>::giveUp(v,index+0);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<typename Func, typename ResultPolicy=
	typename DefaultReturnPolicy<typename ReturnTypeDetector<Func>::RET>::policy
>
class DirectCallFunction
{
public:
	static SQInteger Dispatch(HSQUIRRELVM v)
	{
		void* p = nullptr;
		CAPI_VERIFY(sq_getuserpointer(v, -1, &p));
		MCD_ASSUME(p);
		Func func = Func(p);
		return Call<ResultPolicy>(*func, v, 2);
	}
};

}	// namespace Binding
}	// namespace MCD

#undef CAPI_VERIFY
#undef CHECK_ARG

#endif	// __MCD_CORE_BINDING_BINDING__
