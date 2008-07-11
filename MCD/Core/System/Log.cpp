#include "Pch.h"
#include "Log.h"
#include <iostream>
#include <stdarg.h>

#ifdef MCD_CYGWIN
#	include "StrUtility.h"
#endif

namespace MCD {

static std::wostream* gOutStream = nullptr;

void Log::start(std::wostream* os)
{
	MCD_ASSUME(os != nullptr);
	stop();
	gOutStream = os;
}

void Log::write(const wchar_t* msg)
{
	if(!gOutStream)
		return;
	(*gOutStream) << msg;
}

// _ALLOCA_S_THRESHOLD is a macro that already defined in Visual Studio <malloc.h>
#ifndef MCD_VC
#	define _ALLOCA_S_THRESHOLD 1024*2
#	define _vsnwprintf vsnwprintf
#endif

void Log::format(const wchar_t* fmt, ...)
{
	MCD_ASSUME(fmt != nullptr);
	if(!gOutStream)
		return;

	va_list argList;
	va_start(argList, fmt);

	// In visual stdio we have to magically - 4 on the count of wchar_t,
	// otherwise malloc is invoked all the time rather than _alloca
	size_t bufCount = _ALLOCA_S_THRESHOLD / sizeof(wchar_t) - 4;
	wchar_t* buf = nullptr;

	// Loop until we allocate a large enough buffer for sprintf.
	// There are other alternatives that eliminate the needs of the loop,
	// for example use _vsctprintf_s on VC and vasprintf on gcc.
	for(;;) {
		buf = (wchar_t*)MCD_STACKALLOCA(sizeof(wchar_t) * bufCount);
		if(!buf)	// Not enough memory! It seems that we cannot do anything further
			return;
		int result = _vsnwprintf(buf, bufCount, fmt, argList);
		if(result >= 0) {
			gOutStream->write(buf, result);
			MCD_STACKFREE(buf);
			return;
		}
		MCD_STACKFREE(buf);
		bufCount *= 2;
	}
}

void Log::stop()
{
	delete gOutStream;
	gOutStream = nullptr;
}

}	// namespace MCD
