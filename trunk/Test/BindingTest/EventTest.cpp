#include "Pch.h"
#include "../../MCD/Binding/Binding.h"
#include "../../3Party/jkbind/Declarator.h"

using namespace MCD;

namespace {

class Button
{
public:
	Button() : posX(0), posY(0) {}

	// Declaration of the event. All events belongs to the instances of the classes.
	script::Event<void,				// Return type of the event - what c++ expects to get from script.
		script::objNoCare, Button*,	// First policy of the pushing to script, and first argument type.
		script::plain, int,			// Second policy of the pushing to script, and second argument type.
		script::plain, int			// Third policy of the pushing to script, and third argument type.
	> onMousePressed;				// Name of the event.

	void mouse(int x, int y)
	{
		// Checking if the event is assigned from script, calling of non-assigned event is prohibited!
		if(onMousePressed.isAssigned()) {
			// Calling of event is VERY simple - just passing your declared argumens into "Call"
			onMousePressed.call(this, x, y);
		}
	}

	sal_notnull static Button* defaultButton()
	{
		static Button b;
		return &b;
	}

	int posX, posY;
};	// Button

}	// namespace

namespace script {

SCRIPT_CLASS_DECLAR(Button);

SCRIPT_CLASS_REGISTER_NAME(Button, "Button")
	.enableGetset(L"Button")
	.scriptEvent(L"onMousePressed", &Button::onMousePressed)
	.staticMethod<objNoCare>(L"defaultButton", &Button::defaultButton)
	.getset(L"x", &Button::posX)
	.getset(L"y", &Button::posY)
;}

}	// namespace script

TEST(EventBindingTest)
{
	ScriptVM vm;
	script::VMCore* v = (script::VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));
	script::ClassTraits<Button>::bind(v);

	vm.runScript(L"\
		button <- Button.defaultButton();\
		button.onMousePressed().setHandler(\
			function(sender, x, y) {\
				sender.x = x;\
				sender.y = y;\
			}\
		);\
	");

	Button& b = *Button::defaultButton();
	CHECK_EQUAL(0, b.posX);
	CHECK_EQUAL(0, b.posY);

	b.mouse(10, 20);
	CHECK_EQUAL(10, b.posX);
	CHECK_EQUAL(20, b.posY);

	b.mouse(123, 456);
	CHECK_EQUAL(123, b.posX);
	CHECK_EQUAL(456, b.posY);
}

#include "../../MCD/Binding/Entity.h"
#include "../../MCD/Binding/System.h"
#include "../../MCD/Binding/ScriptComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/StrUtility.h"
#include <fstream>

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

	mgr.vm.runScript(L"c1 <- loadScript(\"test.nut\");");
	mgr.vm.runScript(L"c2 <- loadScript(\"test.nut\");");

	mgr.vm.runScript(L"print(c1.entity)");
	mgr.vm.runScript(L"rootEntity.addComponent(c1);");
	mgr.vm.runScript(L"print(c1.entity)");

	mgr.vm.runScript(L"for(local i=0; i<10000; ++i) updateAllScriptComponent();");
}
