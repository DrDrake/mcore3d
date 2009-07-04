#ifndef __MCD_CORE_SYSTEM_UTILITY__
#define __MCD_CORE_SYSTEM_UTILITY__

#include "../ShareLib.h"
#include "Platform.h"
#include <stdexcept>

#ifdef MCD_VC
#	include <stdlib.h>	// For _countof in MSVC
#endif

/*!	\file Utility.h
	Contains some small but useful utilities.
 */

namespace MCD {

//! Throw an std::runtime_error if the input parameter is null.
MCD_CORE_API void throwIfNull(
	sal_maybenull const void* pointerToCheck,
	sal_in_z sal_notnull const char* message = "Null pointer"
	) throw(std::runtime_error);

/*!	Throw an std::runtime_error with the system error message.
	The format of the message is as follow:
	\em prefixMessage Reason:"system error message"
 */
#ifdef MCD_VC
__declspec(noreturn)
#endif
MCD_CORE_API void throwSystemErrorMessage(
	sal_in_z sal_notnull const char* prefixMessage
	) throw(std::runtime_error);

#ifdef MCD_VC
#	define MCD_FOREACH(Var, Container) \
	for each(Var in Container)
#elif defined(MCD_GCC)

template<class T> struct ForeachTraits {
	typedef typename T::iterator iterator;
};
template<class T> struct ForeachTraits<const T> {
	typedef typename T::const_iterator iterator;
};

#	define MCD_FOREACH(Var, Container) \
	if(bool _breaked_ = false) {} else \
	for(MCD::ForeachTraits<__typeof__(Container)>::iterator _It_ = (Container).begin(), _ItEnd_= (Container).end(); _It_ != _ItEnd_ && (_breaked_ = !_breaked_); ++_It_) \
	for(Var = *_It_; _breaked_; _breaked_ = false)
#endif

/*!	Assert checked polymorphic downcast.
	Use this when your runtime logic is mean to give convertable types, but
	you want to detect any logical error in debug mode.
 */
template<class Target, class Source>
MCD_INLINE2 Target polymorphic_downcast(Source* x) {
	MCD_ASSERT( dynamic_cast<Target>(x) == x );
	return static_cast<Target>(x);
}
template<class Target, class Source>
MCD_INLINE2 Target polymorphic_downcast(Source& x) {
	MCD_ASSERT( &(dynamic_cast<Target>(x)) == &x );
	return static_cast<Target>(x);
}

/*!	Macro to get the count of element of an array
	For the Vsiaul Studio, in use the provided _countof macro defined in
	stdlib.h, which can prevent many miss use of it.
	\sa http://blogs.msdn.com/the1/archive/2004/05/07/128242.aspx
 */
#ifdef MCD_VC
#	define MCD_COUNTOF(x) _countof(x)
#else
	template<class T> class CountofHelper {};
	template<class T, size_t N> struct CountofHelper<T[N]> { static const size_t value = N; };
//#	define MCD_COUNTOF(x) (::MCD::CountofHelper<typeof(x)>::value)
#	define MCD_COUNTOF(x) (sizeof(x) / sizeof(typeof(x[0])))
#endif

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_UTILITY__
