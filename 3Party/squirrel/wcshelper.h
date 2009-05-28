/*	see copyright notice in squirrel.h */
#ifndef _SQWCS_HELPER_H_
#define _SQWCS_HELPER_H_

#include "squirrel.h"

#if defined(SQUNICODE) && !defined(_MSC_VER)

#include <wchar.h>	// For wcslen
#include <wctype.h>	// For iswprint
#include <stdarg.h>	// For va_start

#ifdef __cplusplus
//extern "C" {
#endif

static inline int vswprintf(wchar_t* str, const wchar_t* format, va_list va) {
	return vswprintf(str, scstrlen(str), format, va);
}

static inline int swprintf(wchar_t* buffer, const wchar_t* format, ...) {
	va_list args;
    va_start(args, format);
	int ret = vswprintf(buffer, format, args);
	va_end(args);
	return ret;
}

static int _wtoi(const wchar_t* str) {
	int ret = 0;
	wscanf(str, "%i", &ret);
	return ret;
}

#ifdef __cplusplus
//} /*extern "C"*/
#endif

#endif /* SQUNICODE */

#endif /* _SQWCS_HELPER_H_ */
