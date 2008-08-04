#ifndef __MCD_CORE_SYSTEM_UTILITY__
#define __MCD_CORE_SYSTEM_UTILITY__

#include "../ShareLib.h"
#include "Platform.h"
#include <stdexcept>

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
	for(MCD::ForeachTraits<__typeof__(Container)>::iterator _It_ = (Container).begin(), _ItEnd_= (Container).end(); _It_ != _ItEnd_; ++_It_) \
		for(bool _foreach_continue = true; _foreach_continue; ) \
		for(Var = *_It_; _foreach_continue; _foreach_continue = false)
#endif

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_UTILITY__
