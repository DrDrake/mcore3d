#ifndef __MCD_COMPONENT_SHARELIB__
#define __MCD_COMPONENT_SHARELIB__

#ifndef _WIN32
#	define MCD_COMPONENT_API
#else
#	if defined(MCD_Component_EXPORTS)
#		define MCD_COMPONENT_API __declspec(dllexport)
#	else
#		define MCD_COMPONENT_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_COMPONENT_SHARELIB__
