#include "Pch.h"
#include "Framework.h"
#include "../Core/System/Resource.h"
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

static Resource* load_Framework(Framework& f, const char* path, int blockIteration, int priority) {
	return f.resourceManager().load(path, blockIteration, priority).get();
}

SCRIPT_CLASS_DECLAR(Framework);
SCRIPT_CLASS_REGISTER(Framework)
	.declareClass<Framework>("Framework")
	.staticMethod<objNoCare>("getSingleton", singleton_Framework)
	.method("initWindow", (bool (Framework::*)(const char*))(&Framework::initWindow))
	.method("addFileSystem", &Framework::addFileSystem)
	.method("removeFileSystem", &Framework::removeFileSystem)
	.varGet("rootEntity", &Framework::rootEntity)
	.varGet("systemEntity", &Framework::systemEntity)
	.varGet("sceneLayer", &Framework::sceneLayer)
	.varGet("guiLayer", &Framework::guiLayer)
	.varGet<objNoCare>("input", &input_Framework)
	.varGet("dt", &Framework::dt)
	.varGet("fps", &Framework::fps)
	.method("update", &update_Framework)
	.method("_load", &load_Framework)
	.runScript("Framework.load<-function(path,blockIteration=-1,priority=0){this._load(path,blockIteration,priority);}")
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
