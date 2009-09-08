/*	see copyright notice in squirrel.h */
#ifndef _SQPCHEADER_H_
#define _SQPCHEADER_H_

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new>
//squirrel stuff
#include "../squirrel.h"
#include "sqobject.h"
#include "sqstate.h"

#include <wchar.h>	// For wcstoul
#include <wctype.h>	// For is digit etc...

#endif //_SQPCHEADER_H_
