#include "Pch.h"
#include "Binding.h"
#include "../Core/System/Platform.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/MemoryProfiler.h"
#include "../../3Party/jkbind/VMCore.h"
#include "../../3Party/jkbind/detail/Classes.h"
#include "../../3Party/squirrel/sqstdio.h"
#include "../../3Party/squirrel/sqstdmath.h"
#include "../../3Party/squirrel/sqstdstring.h"
#include "../../3Party/squirrel/sqstdsystem.h"
#include <map>
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

extern void registerSystemBinding(script::VMCore* v);
extern void registerMathBinding(script::VMCore* v);
extern void registerRenderBinding(script::VMCore* v);
extern void registerEntityBinding(script::VMCore* v);
extern void registerComponentQueueBinding(script::VMCore* v);
extern void registerAudioComponentBinding(script::VMCore* v);
extern void registerInputComponentBinding(script::VMCore* v);
extern void registerPhysicsComponentBinding(script::VMCore* v);
extern void registerRenderComponentBinding(script::VMCore* v);
extern void registerScriptComponentBinding(script::VMCore* v);

static void onCompileError(HSQUIRRELVM v,const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
{
	wprintf(L"Compile error: \"%s\" at line %i, column %i\n", desc, line, column);
}

struct TypeInfo
{
	const std::type_info& typeInfo;

	MCD_IMPLICIT TypeInfo(const std::type_info& t) : typeInfo(t) {}

	bool operator<(const TypeInfo& rhs) const {
		return typeInfo.before(rhs.typeInfo) > 0;
	}
};	// TypeInfo

typedef std::map<TypeInfo, void*> TypeMap;
static TypeMap gTypeMap;

void associateClassID(const std::type_info& typeInfo, script::ClassID classID)
{
	gTypeMap[typeInfo] = classID;
}

class ScriptVM::Impl
{
public:
	Impl() : vm(cInitialStackSize)
	{
		// Set the binder's associateClassID() function
		script::detail::ClassesManager::associateClassID = &MCD::associateClassID;

		HSQUIRRELVM v = vm.getVM();
		sq_setprintfunc(v, printfunc);

		// Bind the squirrel's standard library
		sq_pushroottable(v);
		sqstd_register_iolib(v);
		sqstd_register_systemlib(v);
		sqstd_register_mathlib(v);
		sqstd_register_stringlib(v);

		// Bind mcore3d
		registerSystemBinding(&vm);
		registerMathBinding(&vm);
		registerRenderBinding(&vm);
		registerEntityBinding(&vm);

		registerComponentQueueBinding(&vm);
		registerAudioComponentBinding(&vm);
		registerInputComponentBinding(&vm);
		registerPhysicsComponentBinding(&vm);
		registerRenderComponentBinding(&vm);
		registerScriptComponentBinding(&vm);
	}

	bool runScript(const wchar_t* script, bool retVal, const wchar_t* scriptName)
	{
		HSQUIRRELVM v = vm.getVM();

		sq_setcompilererrorhandler(v, &onCompileError);
		if(!SQ_SUCCEEDED(sq_compilebuffer(v, script, SQInteger(::wcslen(script)), scriptName, true)))
			return false;
		sq_pushroottable(v);

		if(!SQ_SUCCEEDED(sq_call(v, 1, retVal, true)))
		{
			const SQChar* s;
			sq_getlasterror(v);
			sq_getstring(v, -1, &s);
			if(s)
				sq_getprintfunc(v)(v, s);
			return false;
		}

		return true;
	}

	static const int cInitialStackSize = 64;
	script::VMCore vm;
};	// Impl

ScriptVM::ScriptVM()
	: mImpl(*new Impl())
{
}

ScriptVM::~ScriptVM()
{
	delete &mImpl;
}

bool ScriptVM::runScript(const wchar_t* script, const wchar_t* scriptName, bool retVal)
{
	MemoryProfiler::Scope profiler("ScriptVM::runScript");
	return mImpl.runScript(script, retVal, scriptName);
}

void* ScriptVM::getImplementationHandle()
{
	return mImpl.vm.getVM();
}

}	// namespace MCD

namespace script {
namespace types {

ClassID getClassIDFromTypeInfo(const std::type_info& typeInfo, ClassID original)
{
	MCD::TypeMap::const_iterator i = MCD::gTypeMap.find(typeInfo);
	if(i != MCD::gTypeMap.end())
		return i->second;
	return original;
}

#ifdef SQUNICODE

void push(HSQUIRRELVM v, const char* value)
{
	// TODO: Error handling
	std::wstring s = MCD::strToWStr(value);
	sq_pushstring(v, s.c_str(), -1);
}

// NOTE: This funciton use local variable to store the returning char*
const char* get(TypeSelect<const char*>, HSQUIRRELVM v, int idx)
{
	const wchar_t* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	static std::string buf;
	buf = MCD::wStrToStr(s);
	return buf.c_str();
}

#else

void push(HSQUIRRELVM v, const wchar_t* value)
{
	// TODO: Error handling
	std::string s = MCD::wStrToStr(value);
	sq_pushstring(v, s.c_str(), -1);
}

const wchar_t* get(TypeSelect<const wchar_t*>, HSQUIRRELVM v, int idx)
{
	const char_t* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	static std::wstring buf;
	buf = MCD::strToWStr(s);
	return buf.c_str();
}

#endif

}	// namespace types
}	// namespace script
