#include "../Core/System/Platform.h"

#ifdef MCD_IPHONE
#	include "iPhone/GLESiPhoneInclude.h"
#else
#	include "../../3Party/glew/glew.h"
#endif

#include <iostream>
#include <string>
#include <vector>
