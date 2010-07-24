#ifndef ___SCRIPT_DETAIL_LANGUAGE___
#define ___SCRIPT_DETAIL_LANGUAGE___

#include "../custom/CustomSystem.h"

#if defined(jkSCRIPT_LANGUAGE_IS_SQUIRREL)
//
// language is: squirrel
//
#	include "../../squirrel/squirrel.h"
//
#else
//
// language is: none
//
#	error unsupported script language
//
#endif

#endif//___SCRIPT_DETAIL_LANGUAGE___