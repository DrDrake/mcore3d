#ifndef __MCD_BULLETBINDING_SHARELIB__
#define __MCD_BULLETBINDING_SHARELIB__

#ifndef _WIN32
#	define MCD_BULLETBINDING_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(MCD_BulletBinding_EXPORTS) || defined(__CPP_TEST__)
#		define MCD_BULLETBINDING_API __declspec(dllexport)
#	else
#		define MCD_BULLETBINDING_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_BULLETBINDING_SHARELIB__

#ifdef NDEBUG

#pragma comment(lib, "libbulletdynamics")
#pragma comment(lib, "libbulletcollision")
#pragma comment(lib, "libbulletmathd")
#pragma comment(lib, "MCDCore")

#else

#pragma comment(lib, "libbulletdynamicsd")
#pragma comment(lib, "libbulletcollisiond")
#pragma comment(lib, "libbulletmathd")
#pragma comment(lib, "MCDCored")

#endif
