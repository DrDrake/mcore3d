#include "Pch.h"
#include "Binding.h"
#include "../MCD/Core/System/Platform.h"
#include "../3Party/jkbind/VMCore.h"
#include "../3Party/squirrel/sqstdmath.h"
#include "../3Party/squirrel/sqstdstring.h"
#include "../3Party/squirrel/sqstdsystem.h"
#include <stdarg.h>	// For va_list
#include <stdio.h>	// For vwprintf
#include <string.h>	// For wcslen

#ifndef NDEBUG
#	pragma comment(lib, "jkbindd")
#	pragma comment(lib, "squirreld")
#else
#	pragma comment(lib, "jkbind")
#	pragma comment(lib, "squirrel")
#endif

namespace MCD {

// The default print function for the scripting VM
static void printfunc(HSQUIRRELVM v, const SQChar* s, ...)
{
	MCD_ASSERT(s != nullptr);
	va_list vl;
	va_start(vl, s);
	vwprintf(s, vl);
	va_end(vl);
}

extern void registerEntityBinding(script::VMCore* v);
extern void registerMathBinding(script::VMCore* v);
extern void registerRenderBinding(script::VMCore* v);
extern void registerSystemBinding(script::VMCore* v);

class ScriptVM::Impl
{
public:
	Impl() : vm(cInitialStackSize)
	{
		HSQUIRRELVM v = vm.getVM();
		sq_setprintfunc(v, printfunc);

		// Bind the squirrel's standard library
		sq_pushroottable(v);
		sqstd_register_systemlib(v);
		sqstd_register_mathlib(v);
		sqstd_register_stringlib(v);

		// Bind mcore3d
		registerEntityBinding(&vm);
		registerMathBinding(&vm);
		registerRenderBinding(&vm);
		registerSystemBinding(&vm);
	}

	~Impl()
	{
	}

	void runScript(const wchar_t* script)
	{
		HSQUIRRELVM v = vm.getVM();
		const wchar_t* scriptName = L"tmp";
		sq_compilebuffer(v, script, SQInteger(::wcslen(script)), scriptName, true);
		sq_pushroottable(v);

		if(!SQ_SUCCEEDED(sq_call(v, 1, false, true)))
		{
			const SQChar* s;
			sq_getlasterror(v);
			sq_getstring(v, -1, &s);
			if(s)
				printfunc(v, s);
		}
	}

	static const int cInitialStackSize = 64;
	script::VMCore vm;
};	// Impl

ScriptVM::ScriptVM()
{
	mImpl = new Impl();
}

ScriptVM::~ScriptVM()
{
	delete mImpl;
}

void ScriptVM::runScript(const wchar_t* script)
{
	MCD_ASSUME(mImpl);
	return mImpl->runScript(script);
}

void* ScriptVM::getImplementationHandle()
{
	MCD_ASSUME(mImpl);
	return mImpl->vm.getVM();
}

}	// namespace MCD
