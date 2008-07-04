#ifndef __SGE_RENDER_SHARELIB__
#define __SGE_RENDER_SHARELIB__

#ifndef _WIN32
#	define SGE_RENDER_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(SGE_Render_EXPORTS) || defined(__CPP_TEST__)
#		define SGE_RENDER_API __declspec(dllexport)
#	else
#		define SGE_RENDER_API __declspec(dllimport)
#	endif
#endif

#endif	// __SGE_RENDER_SHARELIB__
