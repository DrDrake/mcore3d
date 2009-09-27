#include "Pch.h"

#   if _MSC_VER > 1400
#   if _DEBUG
#	    pragma comment(lib, "fbxsdk_md2008d.lib")
#   else
#	    pragma comment(lib, "fbxsdk_md2008.lib")
#   endif
#else
#   if _DEBUG
#	    pragma comment(lib, "fbxsdk_md2005d.lib")
#   else
#	    pragma comment(lib, "fbxsdk_md2005.lib")
#   endif
#endif

#pragma comment(lib, "wininet.lib")