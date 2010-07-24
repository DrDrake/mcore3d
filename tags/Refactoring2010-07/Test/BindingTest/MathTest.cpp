#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"

using namespace MCD;

TEST(Math_BindingTest)
{
	ScriptVM vm;
	CHECK(runScriptFile(vm, "MathTest.nut"));
}
