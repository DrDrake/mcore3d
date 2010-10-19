#include "Pch.h"
#include "Framework.h"
#include "../Core/Binding/CoreBindings.h"
#include "../Core/Binding/Declarator.h"
#include "../Core/Binding/VMCore.h"
#include "../Render/Renderer.h"

namespace MCD {
namespace Binding {

static Framework* gFramework = nullptr;

static Framework* singleton_Framework() {
	return gFramework;
}

static InputComponent* input_Framework(Framework& f) {
	return f.inputComponent().get();
}

static bool update_Framework(Framework& f)
{
	Event e;
	f.update(e);
	if(e.Type == Event::Closed)
		return false;

	f.rendererComponent()->render(f.rootEntity());

	return true;
}

SCRIPT_CLASS_DECLAR(Framework);
SCRIPT_CLASS_REGISTER(Framework)
	.declareClass<Framework>("Framework")
	.staticMethod<objNoCare>("getSingleton", singleton_Framework)
	.method("initWindow", (bool (Framework::*)(const char*))(&Framework::initWindow))
	.varGet("rootEntity", &Framework::rootEntity)
	.varGet("systemEntity", &Framework::systemEntity)
	.varGet("sceneLayer", &Framework::sceneLayer)
	.varGet("guiLayer", &Framework::guiLayer)
	.varGet<objNoCare>("input", &input_Framework)
	.varGet("dt", &Framework::dt)
	.varGet("fps", &Framework::fps)
	.method("update", &update_Framework)
;}

void registerFrameworkBinding(VMCore& vm, Framework& framework)
{
	gFramework = &framework;
	Binding::ClassTraits<Framework>::bind(&vm);

	MCD_VERIFY(vm.runScript("framework <- Framework.getSingleton();"));
	MCD_VERIFY(vm.runScript("input <- { _get=function(i) { return ::framework.input; } };"));
}

}	// namespace Binding
}	// namespace MCD
