#ifndef __SGE_CORE_SYSTEM_ERRORCODE__
#define __SGE_CORE_SYSTEM_ERRORCODE__

#include "../ShareLib.h"
#include "Platform.h"
#include <string>

namespace SGE {

//! Get last error code
extern SGE_CORE_API int getLastError();

//! Set last error
extern SGE_CORE_API void setLastError(int errorCode);

//! Convert system error code into a meaningful message
extern SGE_CORE_API std::string getErrorMessage(sal_in_z_opt const char* prefix, int errorCode);

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_ERRORCODE__
