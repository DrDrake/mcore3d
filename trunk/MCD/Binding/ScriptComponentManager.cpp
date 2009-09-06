#include "Pch.h"
#include "ScriptComponentManager.h"
#include "Entity.h"
#include "../Core/System/FileSystem.h"
#include "../Core/System/Log.h"

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
	.rawMethod(xSTRING("doFile"), &scriptComponentManagerDoFile)
;}

}	// namespace script

namespace MCD {

ScriptComponentManager::ScriptComponentManager(IFileSystem& fs)
	: fileSystem(fs)
{
	// Install a println function
	if(!vm.runScript(xSTRING("function println(s) { print(s + \"\\n\"); }")))
		goto OnError;

	// Initialize the file name to class mapping, and the script component factory function
	// TODO: Error handling, ensure the script file does return a class
	if(!vm.runScript(xSTRING("\
		_scriptComponentClassTable <- {};\n\
		gComponentQueue <- ComponentQueue();\n\
		\n\
		// This table hold the ownership of all script component instance\n\
		_scriptComponentInstanceSet <- {};\n\
		\n\
		function _scriptComponentThreadFunction(component) {\n\
			while(true) {\n\
				local e = component.entity;\n\
				if(e != null && e.enabled)\n\
					component.update();\n\
				if(!::suspend(null))\n\
					return;\n\
			}\n\
		}\n\
		\n\
		loadComponent <- function(fileName, ...) {\n\
			local Class;\n\
			if(fileName in _scriptComponentClassTable) {\n\
				Class = _scriptComponentClassTable[fileName];\n\
			} else {\n\
				Class = scriptComponentManager.doFile(fileName);\n\
				Class.thread <- null;	// Adds the thread variable\n\
				Class.sleep <- function(second) {\n\
					if(second < 0) second = 3.0e38;\n\
					local wakeUpTime = ::gFrameTimer.accumulateTime + second;\n\
					gComponentQueue.setItem(wakeUpTime, this);\n\
					::suspend(null);\n\
				}\n\
				Class.wakeup <- function() {\n\
					thread.wakeup(true);\n\
				}\n\
				_scriptComponentClassTable[fileName] <- Class;\n\
			}\n\
			local c;\n\
			if(vargc == 0) {\n\
				c = Class();\n\
			}\n\
			else {\n\
				c = Class.instance();\n\
				local args = array(0);\n\
				args.push(c);\n\
				for(local i=0; i<vargc; ++i)\n\
					args.push(vargv[i]);\n\
				Class.constructor.acall(args);\n\
			}\n\
			c._setScriptHandle();\n\
			c.thread = newthread(_scriptComponentThreadFunction);\n\
			c.thread.call(c);\n\
			_scriptComponentInstanceSet[c] <- c.thread;\n\
			gComponentQueue.setItem(0, c);\n\
			return c;\n\
		}\
		\n\
		function updateAllScriptComponent() {\n\
			local currentTime = ::gFrameTimer.accumulateTime;\n\
			local queueResult = ComponentQueueResult();\n\
			while(true) {\n\
				queueResult = ::gComponentQueue.getItem(currentTime, queueResult.queueNode);\n\
				local component = queueResult.component;\n\
				if(component) {\n\
					if(component.entity.enabled)\n\
						component.wakeup();\n\
					if(!queueResult.queueNode)\n\
						break;\n\
				}\n\
				else\n\
					break;\n\
			}\n\
		}\n\
		\n\
		// Quit all the threads\n\
		function shutdownAllScriptComponent() {\n\
			foreach(key, value in _scriptComponentInstanceSet) {\n\
				value.wakeup(false);\n\
			}\n\
//			_scriptComponentInstanceSet = null;\n\
		}\n\
	")))
		goto OnError;

	// Initialize the file name to class mapping, and the script component factory function
	// TODO: Error handling, ensure the script file does return a class
/*	if(!vm.runScript(L"\
		_scriptComponentClassTable <- {};\n\
		// This table hold the ownership of all script component instance\n\
		_scriptComponentInstanceSet <- {};\n\
		\n\
		loadComponent <- function(fileName, ...) {\n\
			local Class;\n\
			if(fileName in _scriptComponentClassTable) {\n\
				Class = _scriptComponentClassTable[fileName];\n\
			} else {\n\
				Class = scriptComponentManager.doFile(fileName);\n\
				_scriptComponentClassTable[fileName] <- Class;\n\
			}\n\
			local c;\n\
			if(vargc == 0) {\n\
				c = Class();\n\
			}\n\
			else {\n\
				c = Class.instance();\n\
				local args = array(0);\n\
				args.push(c);\n\
				for(local i=0; i<vargc; ++i)\n\
					args.push(vargv[i]);\n\
				Class.constructor.acall(args);\n\
			}\n\
			c._setScriptHandle();\n\
			_scriptComponentInstanceSet[c] <- c;\n\
			return c;\n\
		}\
		\n\
		function updateAllScriptComponent() {\n\
			// TODO: Make this loop more efficient, eliminate the need to call C++ functions\n\
			foreach(key, value in _scriptComponentInstanceSet) {\n\
				try {\n\
					if(key.entity != null && key.entity.enabled)\n\
						key.update();\n\
				} catch(e) {}\n\
			}\n\
		}\n\
	"))
		goto OnError;*/

	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());
	script::VMCore* v_ = (script::VMCore*)sq_getforeignptr(v);
	script::ClassTraits<ScriptComponentManager>::bind(v_);

	// Set a global variable to the script manager.
	sq_pushroottable(v);
	sq_pushstring(v, xSTRING("scriptComponentManager"), -1);
	script::objNoCare::pushResult(v, this);
	sq_rawset(v, -3);
	sq_pop(v, 1);	// Pops the root table

	return;

OnError:
	Log::write(Log::Error, L"ScriptComponentManager construction failed");
}

/*!	Script file reader function for sq_compile
	This funciton is adopted from Squirrel's sqstdio.cpp _io_file_lexfeed_UTF8()
 */
static SQInteger sqReadUtf8(SQUserPointer file) throw()
{
#define READ() inchar = (unsigned char)(is->get()); if(is->gcount() != 1) { return 0; }

	std::istream* is = reinterpret_cast<std::istream*>(file);

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

// TODO: Support byte code loading
static bool loadFile(HSQUIRRELVM v, IFileSystem& fs, const Path& filePath)
{
	std::auto_ptr<std::istream> is(fs.openRead(filePath));
	if(!is.get()) {
		stdSTRING s = xSTRING("Script file: \"");
		s += filePath.getString() + xSTRING("\" not found");
		sq_getprintfunc(v)(v, s.c_str());
		return false;
	}

	return SQ_SUCCEEDED(sq_compile(v, sqReadUtf8, is.get(), filePath.getString().c_str(), true));
}

void ScriptComponentManager::registerRootEntity(Entity& entity)
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(vm.getImplementationHandle());

	// Set a global variable to the root entity.
	sq_pushroottable(v);
	sq_pushstring(v, xSTRING("rootEntity"), -1);
	script::objNoCare::pushResult(v, &entity);
	sq_rawset(v, -3);
	sq_pop(v, 1);	// Pops the root table
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
			const SQChar* s = nullptr;
			sq_getlasterror(v);
			sq_getstring(v, -1, &s);
			if(s)
				sq_getprintfunc(v)(v, s);
		}

		sq_pop(v, 1); // Removes the closure
	}

	return false;
}

// TODO: May be generalized both runScripAsEntity() and runScripAsComponent() into a template function of ScriptVM.
Entity* ScriptComponentManager::runScripAsEntity(const wchar_t* scriptCode, bool scriptKeepOwnership)
{
	using namespace script::types;
	HSQUIRRELVM v = (HSQUIRRELVM)vm.getImplementationHandle();

	if(!vm.runScript(scriptCode, true))
		return nullptr;

	Entity* ret = script::types::get(TypeSelect<Entity*>(), v, -1);
	if(ret && !scriptKeepOwnership)
		sq_setreleasehook(v, -1, NULL);
	return ret;
}

Component* ScriptComponentManager::runScripAsComponent(const wchar_t* scriptCode, bool scriptKeepOwnership)
{
	using namespace script::types;
	HSQUIRRELVM v = (HSQUIRRELVM)vm.getImplementationHandle();

	if(!vm.runScript(scriptCode, true))
		return nullptr;

	Component* ret = script::types::get(TypeSelect<Component*>(), v, -1);
	if(ret && !scriptKeepOwnership)
		sq_setreleasehook(v, -1, NULL);
	return ret;
}

void ScriptComponentManager::updateScriptComponents() {
	(void)vm.runScript(xSTRING("updateAllScriptComponent();"));
}

void ScriptComponentManager::shutdown() {
	(void)vm.runScript(xSTRING("shutdownAllScriptComponent();"));
}

}	// namespace MCD
