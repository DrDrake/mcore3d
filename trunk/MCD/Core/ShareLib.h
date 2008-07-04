#ifndef __SGE_CORE_SHARELIB__
#define __SGE_CORE_SHARELIB__

#ifndef _WIN32
#	define SGE_CORE_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(SGE_Core_EXPORTS) || defined(__CPP_TEST__)
#		define SGE_CORE_API __declspec(dllexport)
#	else
#		define SGE_CORE_API __declspec(dllimport)
#	endif
#endif

#endif	// __SGE_CORE_SHARELIB__
