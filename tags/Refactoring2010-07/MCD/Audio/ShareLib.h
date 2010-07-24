#ifndef __MCD_AUDIO_SHARELIB__
#define __MCD_AUDIO_SHARELIB__

#ifndef _WIN32
#	define MCD_AUDIO_API
#else
// __CPP_TEST__ is for Parasoft C++ test (http://www.parasoft.com/jsp/products/home.jsp?product=CppTest&itemId=47)
#	if defined(MCD_Audio_EXPORTS) || defined(__CPP_TEST__)
#		define MCD_AUDIO_API __declspec(dllexport)
#	else
#		define MCD_AUDIO_API __declspec(dllimport)
#	endif
#endif

#endif	// __MCD_AUDIO_SHARELIB__
