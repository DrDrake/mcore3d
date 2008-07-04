#ifndef __SGE_CORE_SYSTEM_PLATFORMINCLUDE__
#define __SGE_CORE_SYSTEM_PLATFORMINCLUDE__

#include "PlatformIncludeFwd.h"

#if defined(SGE_VC) || defined(SGE_CYGWIN)

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

#if defined(SGE_VC)
#	include <tchar.h>
#endif

#undef NOMINMAX

#include <sys/utime.h>

#endif	// SGE_VC

#ifdef SGE_GCC

#include <sys/time.h>
#include <errno.h>

#endif	// SGE_GCC

#endif	//__SGE_CORE_SYSTEM_PLATFORMINCLUDE__
