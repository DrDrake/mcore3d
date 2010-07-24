#ifndef __MCD_CORE_SYSTEM_STATICASSERT_H_
#define __MCD_CORE_SYSTEM_STATICASSERT_H_

#include "Macros.h"

#ifdef _MSC_VER
#	define MCD_STATIC_ASSERT(expression) _STATIC_ASSERT(expression)
#else

namespace MCD {
	// HP aCC cannot deal with missing names for template value parameters
	template<bool x> struct STATIC_ASSERTION_FAILURE;
	template<> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
	// HP aCC cannot deal with missing names for template value parameters
	template<int x> struct static_assert_test{};
}	// namespace MCD

#define MCD_STATIC_ASSERT(B)							\
typedef ::MCD::static_assert_test<						\
sizeof(::MCD::STATIC_ASSERTION_FAILURE< (bool)( B ) >)>	\
MCD_MACRO_JOIN(sge_static_assert_typedef_, __COUNTER__)

#endif	// MCD_VC

#endif	// __MCD_CORE_SYSTEM_STATICASSERT_H_
