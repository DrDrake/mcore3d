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
}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_WSTR2STR__
