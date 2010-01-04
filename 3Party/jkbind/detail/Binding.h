#ifndef ___SCRIPT_BINDING___
#define ___SCRIPT_BINDING___

#include "StackHandler.h"
#include "ScriptObject.h"
#include "Checking.h"

#include "CommonTypes.h"
#include "DetectReturnType.h"
#include "ReturnPolicies.h"
#include "../custom/CustomTypes.h"

namespace script {
namespace detail {

#if jkDEBUG_SCRIPT
#	define sq_argassert(arg,_index_) if (!match(types::TypeSelect<P##arg>(), v,_index_)) return sq_throwerror(v,xSTRING("Incorrect function argument"))
#	define sq_thisassert(p) if (!(p)) return sq_throwerror(v,xSTRING("Try to call member funciton on null"))
#else
#	define sq_argassert(arg,_index_)
#	define sq_thisassert(p)
#endif

///
/// Return specializations (if not void)
///
template<class RT, typename ReturnPolicy>
struct ReturnSpecialization {

	//
	//Standard functions
	//

	static int Call(RT (*func)(),HSQUIRRELVM v,int /*index*/) {
		RT ret = func();
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1>
	static int Call(RT (*func)(P1),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2>
	static int Call(RT (*func)(P1,P2),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2,typename P3>
	static int Call(RT (*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2,typename P3,typename P4>
	static int Call(RT (*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(RT (*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(RT (*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(RT (*func)(P1,P2,P3,P4,P5,P6,P7),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		RT ret = func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P6>(),v,index + 6)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	//
	// Member Function calls
	//

	template <typename Callee>
	static int Call(Callee & callee,RT (Callee::*func)(),HSQUIRRELVM v,int /*index*/) {
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template <typename Callee,typename P1>
	static int Call(Callee & callee,RT (Callee::*func)(P1),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P7),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P7>(),v,index + 6)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	//
	// Const function calls
	//

	template <typename Callee>
	static int Call(Callee & callee,RT (Callee::*func)() const,HSQUIRRELVM v,int /*index*/) {
		RT ret = (callee.*func)();
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template <typename Callee,typename P1>
	static int Call(Callee & callee,RT (Callee::*func)(P1) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P7) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		RT ret = (callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P7>(),v,index + 6)
			);
		return ReturnPolicy::template pushResult<RT>(v,ret);
	}
};


///
/// Return specialization for void
///
template<typename ReturnPolicy>
struct ReturnSpecialization<void, ReturnPolicy> {

	//
	// Standart function calls
	//

	static int Call(void (*func)(),HSQUIRRELVM v,int /*index*/) {
		func();
		return 0;
	}

	template<typename P1>
	static int Call(void (*func)(P1),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		func(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return 0;
	}

	template<typename P1,typename P2>
	static int Call(void (*func)(P1,P2),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return 0;
	}

	template<typename P1,typename P2,typename P3>
	static int Call(void (*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return 0;
	}

	template<typename P1,typename P2,typename P3,typename P4>
	static int Call(void (*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return 0;
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(void (*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return 0;
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(void (*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return 0;
	}

	template<typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(void (*func)(P1,P2,P3,P4,P5,P6,P7),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		func(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P7>(),v,index + 6)
			);
		return 0;
	}

	//
	// Member function calls
	//

	template<typename Callee>
	static int Call(Callee & callee,void (Callee::*func)(),HSQUIRRELVM,int /*index*/) {
		(callee.*func)();
		return 0;
	}

	template<typename Callee,typename P1>
	static int Call(Callee & callee,void (Callee::*func)(P1),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5,P6,P7),HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P7>(),v,index + 6)
			);
		return 0;
	}

	//
	// Const function calls
	//

	template<typename Callee>
	static int Call(Callee & callee,void (Callee::*func)() const,HSQUIRRELVM,int /*index*/) {
		(callee.*func)();
		return 0;
	}

	template<typename Callee,typename P1>
	static int Call(Callee & callee,void (Callee::*func)(P1) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5,P6) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5)
			);
		return 0;
	}

	template<typename Callee,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
	static int Call(Callee & callee,void (Callee::*func)(P1,P2,P3,P4,P5,P6,P7) const,HSQUIRRELVM v,int index) {
		sq_argassert(1,index + 0);
		sq_argassert(2,index + 1);
		sq_argassert(3,index + 2);
		sq_argassert(4,index + 3);
		sq_argassert(5,index + 4);
		sq_argassert(6,index + 5);
		sq_argassert(7,index + 6);
		(callee.*func)(
			get(types::TypeSelect<P1>(),v,index + 0),
			get(types::TypeSelect<P2>(),v,index + 1),
			get(types::TypeSelect<P3>(),v,index + 2),
			get(types::TypeSelect<P4>(),v,index + 3),
			get(types::TypeSelect<P5>(),v,index + 4),
			get(types::TypeSelect<P6>(),v,index + 5),
			get(types::TypeSelect<P7>(),v,index + 6)
			);
		return 0;
	}
};

#undef sq_argassert

//
// Class traits to determine which functino argument the user want to
// give up the object ownership.
//

template<typename T>
struct GiveUpSpecialization { static void giveUp(HSQUIRRELVM v, int idx) {} };

template<typename T>
struct GiveUpSpecialization<GiveUpOwnership<T> > {
	static void giveUp(HSQUIRRELVM v, int idx) {
		sq_setreleasehook(v, idx, NULL);
	}
};

//
// Standart function callers
//

template<typename ResultPolicy, typename RT>
int Call(RT (*func)(),HSQUIRRELVM v,int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1>
int Call(RT (*func)(P1),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2>
int Call(RT (*func)(P1,P2),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2,typename P3>
int Call(RT (*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2,typename P3,typename P4>
int Call(RT (*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2,typename P3,typename P4,typename P5>
int Call(RT (*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
int Call(RT (*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

template<typename ResultPolicy, typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
int Call(RT (*func)(P1,P2,P3,P4,P5,P6,P7),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	GiveUpSpecialization<P7>::giveUp(v,index+6);
	return ReturnSpecialization<RT, ResultPolicy>::Call(func,v,index);
}

//
// Member function callers
//

template<typename ResultPolicy, typename Callee,typename RT>
int Call(Callee & callee, RT (Callee::*func)(),HSQUIRRELVM v,int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1>
int Call(Callee & callee,RT (Callee::*func)(P1),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2>
int Call(Callee & callee,RT (Callee::*func)(P1,P2),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7),HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	GiveUpSpecialization<P7>::giveUp(v,index+6);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

//
// Const function callers
//

template<typename ResultPolicy, typename Callee,typename RT>
int Call(Callee & callee, RT (Callee::*func)() const,HSQUIRRELVM v,int index) {
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1>
int Call(Callee & callee,RT (Callee::*func)(P1) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2>
int Call(Callee & callee,RT (Callee::*func)(P1,P2) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

template<typename ResultPolicy, typename Callee,typename RT,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
int Call(Callee & callee,RT (Callee::*func)(P1,P2,P3,P4,P5,P6,P7) const,HSQUIRRELVM v,int index) {
	GiveUpSpecialization<P1>::giveUp(v,index+0);
	GiveUpSpecialization<P2>::giveUp(v,index+1);
	GiveUpSpecialization<P3>::giveUp(v,index+2);
	GiveUpSpecialization<P4>::giveUp(v,index+3);
	GiveUpSpecialization<P5>::giveUp(v,index+4);
	GiveUpSpecialization<P6>::giveUp(v,index+5);
	GiveUpSpecialization<P7>::giveUp(v,index+6);
	return ReturnSpecialization<RT, ResultPolicy>::Call(callee,func,v,index);
}

///
/// Direct Call Standard Function handler
///
template<typename Func, typename ResultPolicy=
	typename DefaultReturnPolicy<typename ReturnTypeDetector<Func>::RET>::policy>
class DirectCallFunction
{
public:
	static inline SQInteger Dispatch(HSQUIRRELVM v)
	{
		StackHandler sa(v);
		int paramCount = sa.getParamCount();
		Func func = getStaticFunctionPointer<Func>(v, paramCount);
		return Call<ResultPolicy>(*func, v, 2);
	}
};

///
/// Direct Call Instance Function handler
///
template<typename Callee, typename Func, typename ResultPolicy=
	typename DefaultReturnPolicy<typename ReturnTypeDetector<Func>::RET>::policy>
class DirectCallInstanceMemberFunction
{
public:
	static inline int Dispatch(HSQUIRRELVM v)
	{
		StackHandler sa(v);
		Callee* instance(NULL);
		instance = (Callee*)sa.getInstanceUp(1, 0);
		sq_thisassert(instance);

		int paramCount = sa.getParamCount();
		Func func = getFunctionPointer<Func>(v, paramCount);

		return Call<ResultPolicy, Callee>(*instance, func, v, 2);
	}
};

///
/// Wrapped Instance Function handler, a static function with the first parameter
/// as the "this" pointer will be invoked
///
template<typename Callee, typename Func, typename ResultPolicy=
	typename DefaultReturnPolicy<typename ReturnTypeDetector<Func>::RET>::policy
>
class IndirectCallInstanceMemberFunction
{
public:
	static inline int Dispatch(HSQUIRRELVM v)
	{
		StackHandler sa(v);
		int paramCount = sa.getParamCount();
		Func func = getStaticFunctionPointer<Func>(v, paramCount);
		return Call<ResultPolicy>(*func, v, 1);	// Note the index is 1 instead of 2
	}
};

}   //namespace detail
}	//namespace script

#endif//___SCRIPT_BINDING___
