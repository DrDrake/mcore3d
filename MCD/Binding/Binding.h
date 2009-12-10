#ifndef __MCD_BINDING_BINDING__
#define __MCD_BINDING_BINDING__

#include "ShareLib.h"
#include "../Core/System/Platform.h"
#include <string>

//struct SQVM;
//typedef struct SQVM* HSQUIRRELVM;

namespace MCD {

/*!	Represent a scripting virtual machine.
	Currently it use squirrel as the language, jkbind as the binding.
	There is no plan for making this class to support more than one language.

	\todo Add event, compiled script, calling specific script function etc...
 */
class MCD_BINDING_API ScriptVM
{
public:
	ScriptVM();
	~ScriptVM();

	/*!	Execute a script.
		\param scriptName A name of the script for debugging purpose.
		\param retVal True for remaining the return value on the stack.
	 */
	sal_checkreturn bool runScript(sal_in_z const wchar_t* script, sal_in_z const wchar_t* scriptName=L"unnamed script", bool retVal=false);

	/*!	Execute a script and return the result as a string.
		\note Return empty string if the script has error or the script doesn't return a string.
	 */
	std::wstring runScriptAsString(sal_in_z const wchar_t* script, sal_in_z const wchar_t* scriptName=L"unnamed script");

	//! Get the underlaying scripting VM handle, that is HSQUIRRELVM
	sal_notnull void* getImplementationHandle();

private:
	class Impl;
	Impl& mImpl;
};	// ScriptVM

}	// namespace MCD

namespace script {

typedef void* ClassID;

namespace types {

extern ClassID getClassIDFromTypeInfo(const std::type_info& typeInfo, ClassID original);
/*
template<typename T> class TypeSelect;

#ifdef SQUNICODE
extern void push(HSQUIRRELVM v, const char* value);
extern const char* get(TypeSelect<const char*>, HSQUIRRELVM v, int idx);
#else
extern void push(HSQUIRRELVM v, const wchar_t* value);
extern const wchar_t* get(TypeSelect<const wchar_t*>, HSQUIRRELVM v, int idx);
#endif*/

}	// namespace types

}	// namespace script

#endif	// __MCD_BINDING_BINDING__
