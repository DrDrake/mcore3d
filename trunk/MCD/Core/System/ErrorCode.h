#ifndef __MCD_CORE_SYSTEM_ERRORCODE__
#define __MCD_CORE_SYSTEM_ERRORCODE__

#include "../ShareLib.h"
#include "Platform.h"
#include <string>

namespace MCD {

//! Get last error code
extern MCD_CORE_API int getLastError();

//! Set last error
extern MCD_CORE_API void setLastError(int errorCode);

//! Convert system error code into a meaningful message
extern MCD_CORE_API std::string getErrorMessage(sal_in_z_opt const char* prefix, int errorCode);

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_ERRORCODE__
