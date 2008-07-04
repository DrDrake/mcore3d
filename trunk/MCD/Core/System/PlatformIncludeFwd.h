#ifndef __SGE_CORE_SYSTEM_PLATFORMINCLUDEFWD__
#define __SGE_CORE_SYSTEM_PLATFORMINCLUDEFWD__

#include "Platform.h"

#if defined(SGE_VC)

#include <basetsd.h>	// Type definitions for the basic sized types
typedef int BOOL;
typedef char CHAR;
typedef long LONG;
typedef __int64 LONG64;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef void* LPVOID;
typedef CHAR* LPSTR;
typedef LONG HRESULT;

typedef LONG_PTR LRESULT;
typedef struct HWND__* HWND;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LRESULT (__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct _GUID GUID;
typedef GUID* LPGUID;
typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
typedef struct HACCEL__* HACCEL;

// Intrinsic functions
extern "C" {
	LONG __cdecl _InterlockedIncrement(LONG volatile* pn);
	LONG __cdecl _InterlockedDecrement(LONG volatile* pn);
	LONG __cdecl _InterlockedExchange(LONG volatile* Target, LONG Value);
	LONG __cdecl _InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand);
	LONG64 __cdecl _InterlockedCompareExchange64(LONG64 volatile* Destination, LONG64 ExChange, LONG64 Comperand);
};

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange64)

#endif

#endif	// __SGE_CORE_SYSTEM_PLATFORMINCLUDEFWD__
