#include "Pch.h"
#include "Common.h"
#include "../../Binding/Binding.h"

using namespace MCD;

TEST(Entity_BindingTest)
{
	ScriptVM vm;
	runScriptFile(vm, "EntityTest.nut");
	CHECK(true);
}
