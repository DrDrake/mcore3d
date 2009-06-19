#ifndef __MCD_BINDING_SHARELIB__
#define __MCD_BINDING_SHARELIB__

#ifndef _WIN32
#	define MCD_BINDING_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(MCD_Binding_EXPORTS) || defined(__CPP_TEST__)
#		define MCD_BINDING_API __declspec(dllexport)
#	else
#		define MCD_BINDING_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_BINDING_SHARELIB__
