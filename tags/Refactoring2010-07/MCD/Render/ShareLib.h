#ifndef __MCD_RENDER_SHARELIB__
#define __MCD_RENDER_SHARELIB__

#ifndef _WIN32
#	define MCD_RENDER_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(MCD_Render_EXPORTS) || defined(__CPP_TEST__)
#		define MCD_RENDER_API __declspec(dllexport)
#	else
#		define MCD_RENDER_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_RENDER_SHARELIB__
