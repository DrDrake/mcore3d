#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"

using namespace MCD;

TEST(Entity_BindingTest)
{
	ScriptVM vm;
	CHECK(runScriptFile(vm, "EntityTest.nut"));
}
