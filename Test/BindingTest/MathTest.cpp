#include "Pch.h"
#include "Common.h"
#include "../../Binding/Binding.h"

using namespace MCD;

TEST(Math_BindingTest)
{
	ScriptVM vm;
	runScriptFile(vm, "MathTest.nut");
	CHECK(true);
}
