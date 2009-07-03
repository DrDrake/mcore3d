#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"
#include "../../MCD/Binding/Entity.h"

using namespace MCD;

class ScriptComponentManager
{
public:
	ScriptComponentManager();

	/*!	Set the entity to appear as the root entity in the script VM.
		\note The VM take no ownership of the entity.
		\note Make sure the ScriptComponentManager destroy after the entity.
	 */
	void registerRootEntity(Entity& entity);

	ScriptVM vm;
	ScriptOwnershipHandle scriptOwnershipHandle;
};	// ScriptComponentManager

namespace script {

namespace type {

void addHandleToObject(HSQUIRRELVM v, ScriptComponentManager* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, ScriptComponentManager* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

}	// type

SCRIPT_CLASS_DECLAR(ScriptComponentManager);
SCRIPT_CLASS_REGISTER_NAME(ScriptComponentManager, "ScriptComponentManager")
;}

}	// namespace script

ScriptComponentManager::ScriptComponentManager()
{
	// Initialize the file name to class mapping, and the script component factory function
	// TODO: Error handling, ensure the script file does return a class
	vm.runScript(L"\
		_scriptComponentClassTable <- {};\n\
		// This table hold the ownership of all script component instance\n\
		_scriptComponentInstanceSet <- {};\n\
		\n\
		loadScript <- function(fileName) {\n\
			local Class;\n\
			if(fileName in _scriptComponentClassTable) {\n\
				Class = _scriptComponentClassTable[fileName];\n\
			} else {\n\
				Class = dofile(fileName, true);\n\
				_scriptComponentClassTable[fileName] <- Class;\n\
			}\n\
			local obj = Class();\n\
			_scriptComponentInstanceSet[obj] <- obj;\n\
			return obj;\n\
		}\
		\n\
		function updateAllScriptComponent() {\n\
			foreach(key, value in _scriptComponentInstanceSet) {\n\
				key.update();\n\
			}\n\
		}\n\
	");

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

TEST(ScriptComponentTest)
{
	ScriptComponentManager mgr;
	Entity root;
	mgr.registerRootEntity(root);

	runScriptFile(mgr.vm, "init.nut");
}
