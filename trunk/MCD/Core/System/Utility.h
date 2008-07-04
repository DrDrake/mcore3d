#ifndef __SGE_CORE_SYSTEM_UTILITY__
#define __SGE_CORE_SYSTEM_UTILITY__

#include "../ShareLib.h"
#include "Platform.h"
#include <stdexcept>

/*!	\file Utility.h
	Contains some small but useful utilities.
 */

namespace SGE {

//! Throw an std::runtime_error if the input parameter is null.
SGE_CORE_API void throwIfNull(
	sal_maybenull const void* pointerToCheck,
	sal_in_z sal_notnull const char* message = "Null pointer"
	) throw(std::runtime_error);

/*!	Throw an std::runtime_error with the system error message.
	The format of the message is as follow:
	\em prefixMessage Reason:"system error message"
 */
#ifdef SGE_VC
__declspec(noreturn)
#endif
SGE_CORE_API void throwSystemErrorMessage(
	sal_in_z sal_notnull const char* prefixMessage
	) throw(std::runtime_error);
}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_WSTR2STR__
