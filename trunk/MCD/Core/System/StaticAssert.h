#ifndef __SGE_CORE_SYSTEM_STATICASSERT_H_
#define __SGE_CORE_SYSTEM_STATICASSERT_H_

#include "Macros.h"

#ifdef _MSC_VER
#	define SGE_STATIC_ASSERT(expression) _STATIC_ASSERT(expression)
#else

namespace SGE {
	// HP aCC cannot deal with missing names for template value parameters
	template<bool x> struct STATIC_ASSERTION_FAILURE;
	template<> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
	// HP aCC cannot deal with missing names for template value parameters
	template<int x> struct static_assert_test{};
}	// namespace SGE

#define SGE_STATIC_ASSERT(B)							\
typedef ::SGE::static_assert_test<						\
sizeof(::SGE::STATIC_ASSERTION_FAILURE< (bool)( B ) >)>	\
SGE_MACRO_JOIN(sge_static_assert_typedef_, __COUNTER__)

#endif	// SGE_VC

#endif	// __SGE_CORE_SYSTEM_STATICASSERT_H_
