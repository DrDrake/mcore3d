#include "Pch.h"
#include "Log.h"
#include "Utility.h"
#include <iostream>
#include <stdarg.h>

#ifdef MCD_CYGWIN
#	include "StrUtility.h"
#endif

namespace MCD {

static std::ostream* gOutStream = nullptr;
static Log::Level gLogLevel = Log::Level(Log::Error | Log::Warn | Log::Info);

static const char* cPrefixTable[5] =
{
	"",
	"Error: ",
	"Warn:  ",
	"",
	"Info:  ",
};

void Log::start(std::ostream* os)
{
	MCD_ASSUME(os != nullptr);
	stop();
	gOutStream = os;
}

void Log::setLevel(Level level)
{
	gLogLevel = level;
}

void Log::write(Level level, const char* msg)
{
	if(!gOutStream || !(gLogLevel & level))
		return;
	MCD_ASSUME(uint(level) < MCD_COUNTOF(cPrefixTable));
	(*gOutStream) << cPrefixTable[level] << msg << std::endl;
}

// _ALLOCA_S_THRESHOLD is a macro that already defined in Visual Studio <malloc.h>
#ifndef MCD_VC
#	define _ALLOCA_S_THRESHOLD 1024*2
#endif

void Log::format(Level level, const char* fmt, ...)
{
	MCD_ASSUME(fmt != nullptr);
	if(!gOutStream || !(gLogLevel & level))
		return;

	MCD_ASSUME(uint(level) < MCD_COUNTOF(cPrefixTable));
	va_list argList;
	va_start(argList, fmt);

	// In visual stdio we have to magically - 4 on the count of char,
	// otherwise malloc is invoked all the time rather than _alloca
	size_t bufCount = _ALLOCA_S_THRESHOLD / sizeof(char) - 4;
	char* buf = nullptr;

	// Loop until we allocate a large enough buffer for sprintf.
	// There are other alternatives that eliminate the needs of the loop,
	// for example use _vsctprintf_s on VC and vasprintf on gcc.
	for(;;) {
		buf = (char*)MCD_STACKALLOCA(sizeof(char) * bufCount);
		if(!buf)	// Not enough memory! It seems that we cannot do anything further
			return;
		int result = vsprintf(buf, /*bufCount,*/ fmt, argList);
		if(result >= 0) {
			(*gOutStream) << cPrefixTable[level];
			gOutStream->write(buf, result);
			(*gOutStream) << std::endl;
			MCD_STACKFREE(buf);
			return;
		}
		MCD_STACKFREE(buf);
		bufCount *= 2;
	}
}

void Log::stop(bool destroyStream)
{
	if(destroyStream)
		delete gOutStream;
	gOutStream = nullptr;
}

}	// namespace MCD
