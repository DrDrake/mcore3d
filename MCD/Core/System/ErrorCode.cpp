#include "Pch.h"
#include "ErrorCode.h"
#include "PlatformInclude.h"
#include <errno.h>
#include <string>

namespace SGE {

int getLastError()
{
#ifdef SGE_VC
	return ::GetLastError();
#else
	return errno;
#endif
}

void setLastError(int errorCode)
{
#ifdef SGE_VC
	::SetLastError(errorCode);
#else
	errno = errorCode;
#endif
}

std::string getErrorMessage(sal_in_z_opt const char* prefix, int errorCode)
{
	std::string str(prefix == nullptr ? "" : prefix);

#ifdef SGE_VC
	LPVOID lpMsgBuf = nullptr;
	::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
		(LPSTR)&lpMsgBuf,
		0,
		nullptr
	);

	// If lpMsgBuf is null it means the systemErrorCode is an invalid one
	if(lpMsgBuf) {
		str += static_cast<LPCSTR>(lpMsgBuf);
		::LocalFree(lpMsgBuf);	// Free the buffer
	}

#else
	str += std::strerror(errorCode);
#endif

	return str;
}

}	// namespace SGE
