#include "Pch.h"
#include "Binding.h"
#include "../Core/System/Platform.h"
#include "../../3Party/jkbind/VMCore.h"
#include "../../3Party/squirrel/sqstdio.h"
#include "../../3Party/squirrel/sqstdmath.h"
#include "../../3Party/squirrel/sqstdstring.h"
#include "../../3Party/squirrel/sqstdsystem.h"
#include <stdarg.h>	// For va_list
#include <stdio.h>	// For vwprintf
#include <string.h>	// For wcslen

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "jkbindd")
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "jkbind")
#		pragma comment(lib, "squirrel")
#	endif
#endif

namespace MCD {

// The default print function for the scripting VM
static void printfunc(HSQUIRRELVM v, const SQChar* s, ...)
{
	if(!v || !s)
		return;
	MCD_ASSERT(s != nullptr);
	va_list vl;
	va_start(vl, s);
	vwprintf(s, vl);
	va_end(vl);
}

extern void registerEntityBinding(script::VMCore* v);
extern void registerInputComponentBinding(script::VMCore* v);
extern void registerMathBinding(script::VMCore* v);
extern void registerRenderBinding(script::VMCore* v);
extern void registerScriptComponentBinding(script::VMCore* v);
extern void registerSystemBinding(script::VMCore* v);

static void onCompileError(HSQUIRRELVM v,const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
	wprintf(L"Compile error: \"%s\" at line %i, column %i\n", desc, line, column);
}

class ScriptVM::Impl
{
public:
	Impl() : vm(cInitialStackSize)
	{
		HSQUIRRELVM v = vm.getVM();
		sq_setprintfunc(v, printfunc);

		// Bind the squirrel's standard library
		sq_pushroottable(v);
		sqstd_register_iolib(v);
		sqstd_register_systemlib(v);
		sqstd_register_mathlib(v);
		sqstd_register_stringlib(v);

		// Bind mcore3d
		registerEntityBinding(&vm);
		registerInputComponentBinding(&vm);
		registerMathBinding(&vm);
		registerRenderBinding(&vm);
		registerScriptComponentBinding(&vm);
		registerSystemBinding(&vm);
	}

	~Impl()
	{
	}

	void runScript(const wchar_t* script)
	{
		HSQUIRRELVM v = vm.getVM();
		const wchar_t* scriptName = L"tmp";

		sq_setcompilererrorhandler(v, &onCompileError);
		sq_compilebuffer(v, script, SQInteger(::wcslen(script)), scriptName, true);
		sq_pushroottable(v);

		if(!SQ_SUCCEEDED(sq_call(v, 1, false, true)))
		{
			const SQChar* s;
			sq_getlasterror(v);
			sq_getstring(v, -1, &s);
			if(s)
				sq_getprintfunc(v)(v, s);
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
