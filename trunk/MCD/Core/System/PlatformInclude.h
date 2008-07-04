#ifndef __MCD_CORE_SYSTEM_PLATFORMINCLUDE__
#define __MCD_CORE_SYSTEM_PLATFORMINCLUDE__

#include "PlatformIncludeFwd.h"

#if defined(MCD_VC) || defined(MCD_CYGWIN)

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN			// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501	// We support Windows Server 2003, Windows XP or above (note that Windows 2000 is not supported because of IOCP).
#endif

#define NOMINMAX
#define UNICODE
#define _UNICODE
#include <windows.h>

#if defined(MCD_VC)
#	include <tchar.h>
#endif

#undef NOMINMAX

#include <sys/utime.h>

#endif	// MCD_VC

#ifdef MCD_GCC

#include <sys/time.h>
#include <errno.h>

#endif	// MCD_GCC

#endif	//__MCD_CORE_SYSTEM_PLATFORMINCLUDE__
