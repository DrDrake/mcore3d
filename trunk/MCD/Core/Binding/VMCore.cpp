#include "Pch.h"
#include "VMCore.h"
#include <stdarg.h>	// For va_list

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

namespace MCD {

namespace Binding {

// The default print function for the scripting VM
static void printfunc(HSQUIRRELVM v, const SQChar* s, ...)
{
	if(!v || !s)
		return;
	MCD_ASSERT(s != nullptr);
	va_list vl;
	va_start(vl, s);
	vprintf(s, vl);
	va_end(vl);
}

static void onCompileError(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
	printf("Compile error: \"%s\" at line %i, column %i\n", desc, line, column);
}

void bindEventsToVMCore(VMCore* coreToBind);

VMCore::VMCore(int initialStackSize)
	: mSqvm(0)
	, mState(OPENING)
{
	// Creating vm
	mSqvm = sq_open(initialStackSize);
	sq_setforeignptr(mSqvm, this);

	// Set the print functions
	sq_setprintfunc(mSqvm, printfunc, printfunc);
	sq_setcompilererrorhandler(mSqvm, &onCompileError);

	sq_enabledebuginfo(mSqvm, true);

	// Creating types table
	sq_resetobject(&mClassesTable);
	sq_pushroottable(mSqvm);
	sq_pushstring(mSqvm, "____TYPES_TABLE____", -1);
	sq_newtable(mSqvm);
	CAPI_VERIFY(sq_getstackobj(mSqvm, -1, &mClassesTable));
	sq_addref(mSqvm, &mClassesTable);
	CAPI_VERIFY(sq_newslot(mSqvm, 1, false));
	sq_poptop(mSqvm);

//	bindEventsToVMCore(this);

	mState = ACTIVE;

	MCD_VERIFY(runScript("\
		function printStackInfo(s) {\
			error(\"Func: \" + s.func + \", src: \" + s.src + \", line: \" + s.line + \"\\n\");\
		}\
		function errHandler(v1) {\
			local level = 2;\
			local info;\
			error(\"Callstack:\\n\");\
			while(info = getstackinfos(level++))\
				printStackInfo(info);\
		} seterrorhandler(errHandler);"
	));
}

VMCore::~VMCore()
{
	mState = CLOSING;

	// Releasing types table
	sq_release(mSqvm, &mClassesTable);
	sq_resetobject(&mClassesTable);

	// Destroing vm
	sq_close(mSqvm);

	mSqvm = 0;
}

HSQUIRRELVM VMCore::getVM() const
{
	return mSqvm;
}

bool VMCore::runScript(const char* script, bool retVal, const char* scriptName)
{
	return runScript(mSqvm, script, retVal, scriptName);
}

bool VMCore::runScript(HSQUIRRELVM v, const char* script, bool retVal, const char* scriptName)
{
	const int oldTop = sq_gettop(v);
	(void)oldTop;

	if(!SQ_SUCCEEDED(sq_compilebuffer(v, script, strlen(script), scriptName, true)))
		return false;

	sq_pushroottable(v);

	// TODO: Check if the vm state is suspended after the call (which we don't want)
	if(!SQ_SUCCEEDED(sq_call(v, 1, retVal, true)))
	{
		// TODO: Should we pop the root table and the closure when sq_call failed?
		const char* s = nullptr;
		sq_getlasterror(v);
		sq_getstring(v, -1, &s);

		// TODO: Better error reporting
//		SQStackInfos si;
//		if(SQ_SUCCEEDED(sq_stackinfos(v, -1, &si)))
//			printf("*FUNCTION [%s] %s line [%d]\n", si.funcname, si.source, si.line);

		SQPRINTFUNCTION f = sq_geterrorfunc(v);
		if(s && f)
			f(v, s);

		return false;
	}

	sq_poptop(v);	// Pop the closure
	MCD_ASSERT(oldTop == sq_gettop(v));

	return true;
}

void VMCore::collectGarbage()
{
	sq_collectgarbage(mSqvm);
}

bool runScript(HSQUIRRELVM v, const char* script)
{
	const char* scriptName = "tmp";
	sq_compilebuffer(v, script, SQInteger(scstrlen(script)), scriptName, true);
	sq_pushroottable(v);

	return SQ_SUCCEEDED(sq_call(v, 1, false, true));
}

}	//namespace Binding

}	//namespace MCD
