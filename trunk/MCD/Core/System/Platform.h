#ifndef __MCD_CORE_SYSTEM_PLATFORM__
#define __MCD_CORE_SYSTEM_PLATFORM__

#ifdef _MSC_VER
#	define MCD_VC
#endif

#ifdef __GNUC__
#	define MCD_GCC
#endif

#ifdef _WIN32
#	define MCD_WIN32
#endif

#ifdef __CYGWIN__
#	define MCD_CYGWIN
#endif

#ifndef UNICODE
#	define UNICODE
#endif

#ifndef _UNICODE
#	define _UNICODE
#endif

//////////////////////////////////////////////////////////////////////////
// For Visual Studio

#if defined(MCD_VC)

#ifndef _DEBUG
#	ifndef NDEBUG
#		define NDEBUG
#	endif
#endif

/*!	Defines this macro for template overloads to help ease
	the transition to the more secure C run-time library
 */
#if defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#	undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
// Suppress the 4996 deprecation warning
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

/*!	An optimization on type that will never create an instance.
	The compiler will event generate vtable for classes having pure virtual function (confirmed in VC8)
	Using this macro remove both code size and run-time overhead of unnecessary vtable setting
	\sa http://msdn.microsoft.com/msdnmag/issues/0300/c/
 */
#define MCD_NOVTABLE __declspec(novtable)

//! Declaring aligned variable
#define MCD_VARALIGN(declaration, num) __declspec(align(num)) declaration;

//! Different level of inline attribute
#define MCD_NOINLINE  __declspec(noinline)
#define MCD_INLINE0
#define MCD_INLINE1 inline
#define MCD_INLINE2 __forceinline

//! Annotation
#define MCD_IMPLICIT

// Sized types
typedef __int8	int8_t;
typedef __int16	int16_t;
typedef __int32	int32_t;
typedef __int64	int64_t;

typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64	uint64_t;

// Other types
typedef unsigned int uint;
typedef unsigned char byte_t;
typedef int ssize_t;
typedef unsigned int useconds_t;

// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1601.pdf
// for a description of nullptr
#ifndef nullptr
#	define nullptr 0
#endif

// Suppress warning that really does no harm
#pragma warning (disable : 4100)		// Unreferenced formal parameter
#pragma warning (disable : 4127)		// Conditional expression is constant
#pragma warning (disable : 4201)		// Nameless struct/union
#pragma warning (disable : 4275)		// Non dll-interface class used as base for dll-interface class
#pragma warning (disable : 4290)		// C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#pragma warning (disable : 4503)		// Decorated name length exceeded, name was truncated
#pragma warning (disable : 4510)		// Default constructor could not be generated
#pragma warning (disable : 4512)		// Assignment operator could not be generated
#pragma warning (disable : 4610)		// Type can never be instantiated - user defined constructor required
#pragma warning (disable : 4714)		// Function marked as __forceinline not inlined

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <assert.h>

#ifdef  NDEBUG
#	define MCD_ASSERT(Expression) ((void)0)
#	define MCD_VERIFY(Expression) ((void)(Expression))
#	define MCD_ASSUME(Expression) __assume(Expression)
#else
#	define MCD_ASSERT(Expression) assert(Expression)
#	define MCD_VERIFY(Expression) assert(Expression)
#	define MCD_ASSUME(Expression) { __assume(Expression); MCD_ASSERT(Expression); }
#endif

/*! Use this function to tell the compiler that this function will never return.
	It is useful to suppress warning like:
	\code
	int b;
	switch(a) {
		case 1: b = 2; break;
		case 2: b = 5; break;
		default: noReturn();	// Remove the warning about the variable b not initialized
	}
	\endcode
 */
__declspec(noreturn) inline void noReturn() { assert(false); }

// SAL macros
#include <codeanalysis\sourceannotations.h>
using namespace vc_attributes;
/*!	Standard Annotation Language (SAL)
	\sa http://msdn.microsoft.com/en-us/library/ms182033.aspx
 */
#define SAL(...) [__VA_ARGS__]
#define sal_in __in __notnull								// (input, read only) (cannot be null)
#define sal_in_opt __in_opt __maybenull						// (input, read only) (may be null)
#define sal_in_z __in_z __notnull							// (input, read only) (null terminated) (cannot be null)
#define sal_in_z_opt __in_z_opt __maybenull					// (input, read only) (null terminated) (may be null)
#define sal_out_opt __out_opt __maybenull					// (output, write) (may be null)
#define sal_inout __inout __notnull							// (input/output, read/write) (cannot be null)
#define sal_notnull __notnull								// (cannot be null)
#define sal_maybenull __maybenull							// (may be null)
#define sal_override __override								// (function is overrided from the base one)
#define sal_checkreturn __checkReturn						// (caller must check the return value)
#define sal_in_ecount(count) __in_ecount(count)				// (input, not null, with count as the element count)
#define sal_in_ecount_opt(count) __in_ecount_opt(count)		// (input, not null, with count as the element count) (may be null)
#define sal_out_ecount(count) __out_ecount(count)			// (output, not null, with count as the element count)
#define sal_out_ecount_opt(count) __out_ecount_opt(count)	// (output, not null, with count as the element count) (may be null)

// alloca
#define MCD_STACKALLOCA(size) _malloca(size)
#define MCD_STACKFREE(p) _freea(p)

//////////////////////////////////////////////////////////////////////////
// For GCC

#elif defined(MCD_GCC)

#define MCD_NOVTABLE

//! Declaring aligned variable
#define MCD_CC_VAR_ALIGN(declaration, num) declaration __attribute__((aligned(num)));

//! Different level of inline attribute
#define MCD_NOINLINE __attribute__ ((noinline))
#define MCD_INLINE0
#define MCD_INLINE1 inline
#if !defined(__NO_INLINE__)
#	define MCD_INLINE2 __inline__ __attribute__((always_inline))
#else
#	define MCD_INLINE2 inline
#endif

//! Annotation
#define MCD_IMPLICIT

// Sized types
#include <inttypes.h>
#include <unistd.h>

// Other types
typedef unsigned int uint;
typedef unsigned char byte_t;
#define nullptr __null

#include <assert.h>

#ifdef  NDEBUG
#	define MCD_ASSERT(Expression) ((void)0)
#	define MCD_VERIFY(Expression) ((void)(Expression))
#	define MCD_ASSUME(Expression) ((void)0)
#else
#	define MCD_ASSERT(Expression) assert(Expression)
#	define MCD_VERIFY(Expression) assert(Expression)
#	define MCD_ASSUME(Expression) assert(Expression)
#endif

inline void noReturn() __attribute__((noreturn));
void noReturn() { throw "NoReturn should not be invoked"; }

// SAL macros
#define SAL(...)
#define sal_in
#define sal_in_opt
#define sal_in_z
#define sal_in_z_opt
#define sal_out_opt
#define sal_inout
#define sal_notnull
#define sal_maybenull
#define sal_override
#define sal_checkreturn
#define sal_in_ecount(count)
#define sal_out_ecount(count)

// alloca
#define MCD_STACKALLOCA(size) alloca(size)
#define MCD_STACKFREE(p)

#ifdef MCD_GCC
#include <bits/stringfwd.h>
#include <sstream>
namespace std {
	// Cygwin missing the typedef for std::wstring
	typedef basic_string<wchar_t> wstring;
	typedef basic_ostream<wchar_t> wostream;
	typedef basic_stringstream<wchar_t> wstringstream;
}
#endif

#endif

#endif	// __MCD_CORE_SYSTEM_PLATFORM__
