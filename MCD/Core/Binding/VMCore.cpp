#include "Pch.h"
#include "VMCore.h"
#include "../System/Stream.h"
#include "../../../3Party/squirrel/sqstdio.h"
#include "../../../3Party/squirrel/sqstdmath.h"
#include "../../../3Party/squirrel/sqstdstring.h"
#include "../../../3Party/squirrel/sqstdsystem.h"
#include <iostream>
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

	// Bind the squirrel's standard library
	sq_pushroottable(mSqvm);
	sqstd_register_iolib(mSqvm);
	sqstd_register_systemlib(mSqvm);
	sqstd_register_mathlib(mSqvm);
	sqstd_register_stringlib(mSqvm);

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
		} seterrorhandler(errHandler);\
		function println(s) {\
			print(s); print(\"\\n\");\
		}"
	));
}

VMCore::~VMCore()
{
	mState = CLOSING;

	// Release all thread
	for(ThreadMap::iterator i=mThreadMap.begin(); i!=mThreadMap.end(); ++i)
		sq_release(mSqvm, &i->second);

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

bool VMCore::loadScript(const char* script, int lenInByte, const char* scriptName)
{
	return loadScript(mSqvm, script, lenInByte, scriptName);
}

bool VMCore::loadScript(std::istream& is, int sizeInByte, const char* scriptName)
{
	return loadScript(mSqvm, is, sizeInByte, scriptName);
}

bool VMCore::runScript(const char* script, int lenInByte, bool retVal, bool leftClouseOnStack, const char* scriptName)
{
	return runScript(mSqvm, script, lenInByte, retVal, leftClouseOnStack, scriptName);
}

bool VMCore::runScript(std::istream& is, int sizeInByte, bool retVal, bool leftClouseOnStack, const char* scriptName)
{
	return runScript(mSqvm, is, sizeInByte, retVal, leftClouseOnStack, scriptName);
}

bool VMCore::saveByteCode(std::ostream& os, bool leftClouseOnStack)
{
	return saveByteCode(mSqvm, os, leftClouseOnStack);
}

struct ReadContext
{
	sal_notnull std::istream* is;
	size_t readCount;
	size_t maxCount;
	bool eof() const { return readCount > maxCount; }
};	// ReadContext

static SQInteger sqReadByteCode(SQUserPointer file, SQUserPointer buf, SQInteger size)
{
	ReadContext* ctx = reinterpret_cast<ReadContext*>(file);
	MCD_ASSUME(ctx);

	SQInteger ret = MCD::read(*ctx->is, buf, size);
	ctx->readCount += ret;

	if(ret < 0 || ctx->eof())
		return -1;

	return ret;
}

static SQInteger sqReadUtf8(SQUserPointer file)
{
#define READ() inchar = (unsigned char)(ctx->is->get()); ++ctx->readCount; if(ctx->is->gcount() != 1 || ctx->eof()) { return 0; }

	ReadContext* ctx = reinterpret_cast<ReadContext*>(file);
	MCD_ASSUME(ctx);

	static const SQInteger utf8_lengths[16] =
	{
		1,1,1,1,1,1,1,1,	// 0000 to 0111 : 1 byte (plain ASCII)
		0,0,0,0,			// 1000 to 1011 : not valid
		2,2,				// 1100, 1101 : 2 bytes
		3,					// 1110 : 3 bytes
		4					// 1111 :4 bytes
	};
	static const unsigned char byte_masks[5] = { 0, 0, 0x1f, 0x0f, 0x07 };
	unsigned char inchar;
	SQInteger c = 0;
	READ();
	c = inchar;

	if(c >= 0x80) {
		SQInteger tmp;
		SQInteger codelen = utf8_lengths[c>>4];
		if(codelen == 0) 
			return 0;	// Invalid UTF-8 stream
		tmp = c & byte_masks[codelen];
		for(SQInteger n = 0; n < codelen-1; ++n) {
			tmp <<= 6;
			READ();
			tmp |= inchar & 0x3F;
		}
		c = tmp;
	}
	return c;
#undef READ
}

bool VMCore::loadScript(HSQUIRRELVM v, const char* script, ssize_t len, const char* scriptName)
{
	if(len < 0) len = ssize_t(strlen(script));
	return SQ_SUCCEEDED(sq_compilebuffer(v, script, len, scriptName, true));
}

bool VMCore::loadScript(HSQUIRRELVM v, std::istream& is, ssize_t sizeInByte, const char* scriptName)
{
	uint16_t header = 0;
	if(!MCD::read(is, header))
		return false;

	// Put the header back to the stream
	for(size_t i=sizeof(header); i--;)
		is.unget();

	ReadContext context = { &is, 0, size_t(sizeInByte) };

	if(header == SQ_BYTECODE_STREAM_TAG)	// Load as byte code
	{
		if(SQ_FAILED(sq_readclosure(v, &sqReadByteCode, &context)))
			return printError(v);
	}
	else {	// Load as string
		sq_setcompilererrorhandler(v, &onCompileError);

		if(SQ_FAILED(sq_compile(v, sqReadUtf8, &context, scriptName, true)))
			return printError(v);
	}

	return true;
}

bool VMCore::runScript(HSQUIRRELVM v, const char* script, ssize_t len, bool retVal, bool leftClouseOnStack, const char* scriptName)
{
	const SQInteger oldTop = sq_gettop(v);
	(void)oldTop;

	if(!loadScript(v, script, len, scriptName))
		return false;

	const bool ok = call(v, retVal);

	if(!leftClouseOnStack)
		sq_poptop(v);	// Pop the closure

	MCD_ASSERT(oldTop == sq_gettop(v) - leftClouseOnStack - retVal);

	return ok;
}

bool VMCore::runScript(HSQUIRRELVM v, std::istream& is, ssize_t sizeInByte, bool retVal, bool leftClouseOnStack, const char* scriptName)
{
	const SQInteger oldTop = sq_gettop(v);
	(void)oldTop;

	if(!loadScript(v, is, sizeInByte, scriptName))
		return false;

	const bool ok = call(v, retVal);

	if(!leftClouseOnStack)
		sq_poptop(v);	// Pop the closure

	MCD_ASSERT(oldTop == sq_gettop(v) - leftClouseOnStack - retVal);

	return ok;
}

static SQInteger sqWriteByteCode(SQUserPointer file, SQUserPointer buf, SQInteger size)
{
	std::ostream* os = reinterpret_cast<std::ostream*>(file);
	MCD_ASSUME(os);
	return MCD::write(*os, buf, size);
}

bool VMCore::saveByteCode(HSQUIRRELVM v, std::ostream& os, bool leftClouseOnStack)
{
	if(SQ_FAILED(sq_writeclosure(v, &sqWriteByteCode, &os)))
		return printError(v);

	return true;
}

void VMCore::collectGarbage()
{
	sq_collectgarbage(mSqvm);
}

bool VMCore::call(HSQUIRRELVM v, bool retVal)
{
	sq_pushroottable(v);

	// TODO: Check if the vm state is suspended after the call (which we don't want)
	if(SQ_FAILED(sq_call(v, 1, retVal, true)))
		return printError(v);

	return true;
}

bool VMCore::printError(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);

	// TODO: Should we pop the root table and the closure when sq_call failed?
	const char* s = nullptr;
	sq_getlasterror(v);
	sq_getstring(v, -1, &s);

	// TODO: Better error reporting
//	SQStackInfos si;
//	if(SQ_SUCCEEDED(sq_stackinfos(v, -1, &si)))
//		printf("*FUNCTION [%s] %s line [%d]\n", si.funcname, si.source, si.line);

	SQPRINTFUNCTION f = sq_geterrorfunc(v);
	if(s && f)
		f(v, s);

	sq_settop(v, top);

	return false;
}

HSQUIRRELVM VMCore::allocateThraed()
{
	if(!mFreeThreads.empty()) {
		HSQUIRRELVM ret = mFreeThreads.top();
		mFreeThreads.pop();
		sq_pushobject(mSqvm, mThreadMap[ret]);
		return ret;
	}

	HSQOBJECT thread;
	HSQUIRRELVM newVm = sq_newthread(mSqvm, 64);
	sq_setforeignptr(newVm, this);
	CAPI_VERIFY(sq_getstackobj(mSqvm, -1, &thread));

	mThreadMap[newVm] = thread;
	sq_addref(mSqvm, &thread);

	return newVm;
}

void VMCore::releaseThread(HSQUIRRELVM v)
{
	MCD_ASSERT(SQ_VMSTATE_SUSPENDED != sq_getvmstate(v));
	sq_settop(v, 0);
	mFreeThreads.push(v);
}

void VMCore::scheduleWakeup(HSQUIRRELVM v, float timeToWake, void* userData)
{
	UserData d = { v, userData };
	mSchedule.insert(std::make_pair(timeToWake, d));
}

HSQUIRRELVM VMCore::popScheduled(float currentTime, void** userData)
{
	if(mSchedule.empty()) return nullptr;

	Schedule::iterator i = mSchedule.begin();
	if(i->first > currentTime)
		return nullptr;

	HSQUIRRELVM v = i->second.v;
	if(userData) *userData = i->second.d;

	mSchedule.erase(i);
	MCD_ASSERT(SQ_VMSTATE_SUSPENDED == sq_getvmstate(v));

	return v;
}

float VMCore::currentTime() const
{
	return float(mTimer.get().asSecond());
}

}	// namespace Binding
}	// namespace MCD
