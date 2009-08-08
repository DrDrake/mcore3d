#include "Pch.h"
#include "ScriptComponentManager.h"
#include "Entity.h"
#include "../Core/System/FileSystem.h"

namespace script {

using namespace MCD;

static int scriptComponentManagerDoFile(HSQUIRRELVM v)
{
	// TODO: Parameter type checking
	script::detail::StackHandler sa(v);
	ScriptComponentManager& self = get(types::TypeSelect<ScriptComponentManager&>(), v, 1);
	const wchar_t* filePath = sa.getString(2);
	return self.doFile(filePath, true) ? 1 : 0;
}

SCRIPT_CLASS_DECLAR(ScriptComponentManager);
SCRIPT_CLASS_REGISTER_NAME(ScriptComponentManager, "ScriptComponentManager")
	.rawMethod(L"doFile", &scriptComponentManagerDoFile)
;}

}	// namespace script

namespace MCD {

ScriptComponentManager::ScriptComponentManager(IFileSystem& fs)
	: fileSystem(fs)
{
	// Initialize the file name to class mapping, and the script component factory function
	// TODO: Error handling, ensure the script file does return a class
	vm.runScript(L"\
		_scriptComponentClassTable <- {};\n\
		// This table hold the ownership of all script component instance\n\
		_scriptComponentInstanceSet <- {};\n\
		\n\
		loadComponent <- function(fileName) {\n\
			local Class;\n\
			if(fileName in _scriptComponentClassTable) {\n\
				Class = _scriptComponentClassTable[fileName];\n\
			} else {\n\
				Class = scriptComponentManager.doFile(fileName);\n\
				_scriptComponentClassTable[fileName] <- Class;\n\
			}\n\
			local obj = Class();\n\
			_scriptComponentInstanceSet[obj] <- obj;\n\
			return obj;\n\
		}\
		\n\
		function updateAllScriptComponent() {\n\
			// TODO: Make this loop more efficient, eliminate the need to call C++ functions\n\
			foreach(key, value in _scriptComponentInstanceSet) {\n\
				if(key.entity != null)\n\
					key.update();\n\
			}\n\
		}\n\
	");

	// Install a println function
	vm.runScript(L"function println(s) { print(s + \"\\n\"); }");

	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());
	script::VMCore* v_ = (script::VMCore*)sq_getforeignptr(v);
	script::ClassTraits<ScriptComponentManager>::bind(v_);

	// Set a global variable to the script manager.
	sq_pushroottable(v);
	sq_pushstring(v, L"scriptComponentManager", -1);
	script::objNoCare::pushResult(v, this);
	sq_rawset(v, -3);
	sq_pop(v, 1);	// Pops the root table
}

/*!	Script file reader function for sq_compile
	This funciton is adopted from Squirrel's sqstdio.cpp _io_file_lexfeed_UTF8()
 */
static SQInteger sqReadUtf8(SQUserPointer file) throw()
{
#define READ() inchar = unsigned char(is->get()); if(is->gcount() != 1) { return 0; }

	std::istream* is = reinterpret_cast<std::istream*>(file);

	static const SQInteger utf8_lengths[16] =
	{
		1,1,1,1,1,1,1,1,	// 0000 to 0111 : 1 byte (plain ASCII)
		0,0,0,0,			// 1000 to 1011 : not valid
		2,2,				// 1100, 1101 : 2 bytes
		3,					// 1110 : 3 bytes
		4					// 1111 :4 bytes
	};
	static unsigned char byte_masks[5] = { 0, 0, 0x1f, 0x0f, 0x07 };
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

// TODO: Support byte code loading
static bool loadFile(HSQUIRRELVM v, IFileSystem& fs, const Path& filePath)
{
	std::auto_ptr<std::istream> is(fs.openRead(filePath));
	if(!is.get()) {
		std::wstring s = L"Script file: \"";
		s += filePath.getString() + L"\" not found";
		sq_getprintfunc(v)(v, s.c_str());
		return false;
	}

	return SQ_SUCCEEDED(sq_compile(v, sqReadUtf8, is.get(), filePath.getString().c_str(), true));
}

// TODO: Support byte code loading
bool ScriptComponentManager::doFile(const Path& filePath, bool retval)
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());

	if(loadFile(v, fileSystem, filePath))
	{
//		sq_push(v, -2);			// Instead of pushing the original self, we
		sq_pushroottable(v);	// push the root table for use with dofile

		if(SQ_SUCCEEDED(sq_call(v, 1, retval, SQTrue))) {
			sq_remove(v, retval ? -2 : -1);	// Removes the closure
			return true;
		} else {
			const SQChar* s;
			sq_getlasterror(v);
			sq_getstring(v, -1, &s);
			if(s)
				sq_getprintfunc(v)(v, s);
		}

		sq_pop(v, 1); // Removes the closure
	}

	return false;
}

void ScriptComponentManager::updateScriptComponents()
{
//	vm.runScript(L"updateAllScriptComponent();");
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());

	sq_pushroottable(v);
	sq_pushstring(v, L"updateAllScriptComponent", -1);
	sq_get(v, -2);			// Get the function from the root table
	sq_pushroottable(v);	// 'this' (function environment object)
	sq_call(v, 1, false, true);
	sq_pop(v, 2);			// Pops the roottable and the function
}

void ScriptComponentManager::registerRootEntity(Entity& entity)
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());

	// Set a global variable to the root entity.
	sq_pushroottable(v);
	sq_pushstring(v, L"rootEntity", -1);
	script::objNoCare::pushResult(v, &entity);
	sq_rawset(v, -3);
	sq_pop(v, 1);	// Pops the root table
}

}	// namespace MCD
