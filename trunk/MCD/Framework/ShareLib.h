#ifndef __MCD_FRAMEWORK_SHARELIB__
#define __MCD_FRAMEWORK_SHARELIB__

#ifndef _WIN32
#	define MCD_FRAMEWORK_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(MCD_Framework_EXPORTS) || defined(__CPP_TEST__)
#		define MCD_FRAMEWORK_API __declspec(dllexport)
#	else
#		define MCD_FRAMEWORK_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_FRAMEWORK_SHARELIB__
