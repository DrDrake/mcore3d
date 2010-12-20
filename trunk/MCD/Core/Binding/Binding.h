#ifndef __MCD_CORE_BINDING_BINDING__
#define __MCD_CORE_BINDING_BINDING__

#include "../ShareLib.h"
#include "ReturnPolicies.h"
#include "ReturnTypeDetector.h"
#include "../../../3Party/squirrel/squirrel.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))
#define CHECK_ARG(arg) if(!match(TypeSelect<P##arg>(), v, index+arg-1)) MCD_ASSERT(false);

#ifdef MCD_VC
#ifdef MCD_WIN32
#	ifndef NDEBUG
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "squirrel")
#	endif
#elif defined(MCD_WIN64)
#	ifndef NDEBUG
#		pragma comment(lib, "squirreld_x64")
#	else
#		pragma comment(lib, "squirrel_x64")
#	endif
#endif
#endif

namespace MCD {
namespace Binding {

class ClassesManager;

/// Return specializations for non-void function
template<class RT, class ReturnPolicy>
struct ReturnSpecialization
{
// Static functions:
	static SQInteger Call(RT (*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = func();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1>
	static SQInteger Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2>
	static SQInteger Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3>
	static SQInteger Call(RT (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3,class P4>
	static SQInteger Call(RT (*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(RT (*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(RT (*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(RT (*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		RT ret = func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

// Member functions:
	template<class Callee>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

// Const member functions:
	template<class Callee>
	static SQInteger Call(Callee & callee, RT (Callee::*func)() const, HSQUIRRELVM v, int index) {
		(void)index;
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(Callee & callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		RT ret = (callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return ReturnPolicy::template pushResult<RT>(v, ret);
	}
};	// ReturnSpecialization, non-void function

/// Return specialization for void function
template<class ReturnPolicy>
struct ReturnSpecialization<void, ReturnPolicy>
{
// Static function:
	static SQInteger Call(void (*func)(), HSQUIRRELVM v, int index) {
		func();
		return 0;
	}

	template<class P1>
	static SQInteger Call(void (*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		func(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class P1,class P2>
	static SQInteger Call(void (*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class P1,class P2,class P3>
	static SQInteger Call(void (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return 0;
	}

	template<class P1,class P2,class P3,class P4>
	static SQInteger Call(void (*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return 0;
	}

	template<class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(void (*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return 0;
	}

	template<class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(void (*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return 0;
	}

	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(void (*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		func(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return 0;
	}

	// Member functions:
	template<class Callee>
	static SQInteger Call(Callee & callee, void (Callee::*func)(), HSQUIRRELVM v, int index) {
		(void)index;
		(callee.*func)();
		return 0;
	}

	template<class Callee, class P1>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1), HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class Callee, class P1,class P2>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return 0;
	}

// Const member functions:
	template<class Callee>
	static SQInteger Call(Callee & callee, void (Callee::*func)() const, HSQUIRRELVM v, int index) {
		(void)index;
		(callee.*func)();
		return 0;
	}

	template<class Callee, class P1>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0)
		);
		return 0;
	}

	template<class Callee, class P1,class P2>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5,P6) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5)
		);
		return 0;
	}

	template<class Callee, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	static SQInteger Call(Callee & callee, void (Callee::*func)(P1,P2,P3,P4,P5,P6,P7) const, HSQUIRRELVM v, int index) {
		CHECK_ARG(1); CHECK_ARG(2); CHECK_ARG(3); CHECK_ARG(4); CHECK_ARG(5); CHECK_ARG(6); CHECK_ARG(7);
		(callee.*func)(
			get(TypeSelect<P1>(), v, index + 0),
			get(TypeSelect<P2>(), v, index + 1),
			get(TypeSelect<P3>(), v, index + 2),
			get(TypeSelect<P4>(), v, index + 3),
			get(TypeSelect<P5>(), v, index + 4),
			get(TypeSelect<P6>(), v, index + 5),
			get(TypeSelect<P7>(), v, index + 6)
		);
		return 0;
	}
};	// ReturnSpecialization, void function

// Call(), aims to decompose the incomming function's reutrn and parameter types

// Static function callers
template<class ResultPolicy, class RT>
SQInteger Call(RT (*func)(), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1>
SQInteger Call(RT (*func)(P1), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2>
SQInteger Call(RT (*func)(P1,P2), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3>
SQInteger Call(RT (*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3,class P4>
SQInteger Call(RT (*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3,class P4,class P5>
SQInteger Call(RT (*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3,class P4,class P5,class P6>
SQInteger Call(RT (*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

template<class ResultPolicy, class RT, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
SQInteger Call(RT (*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func, v, index);
}

// Member function callers
template<class ResultPolicy, class Callee, class RT>
SQInteger Call(Callee& callee, RT (Callee::*func)(), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5,class P6>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7), HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

// Const member function callers
template<class ResultPolicy, class Callee, class RT>
SQInteger Call(Callee& callee, RT (Callee::*func)() const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5,class P6>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6) const, HSQUIRRELVM v, int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee, func, v, index);
}

template<class ResultPolicy, class Callee, class RT, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
SQInteger Call(Callee& callee, RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7) const, HSQUIRRELVM v, int index) {
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
		if(!instance)
			return sq_throwerror(v, "Try to call member funciton on null");

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

#endif	// __MCD_CORE_BINDING_BINDING__
