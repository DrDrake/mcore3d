#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"
#include "../../3Party/jkbind/Declarator.h"

using namespace MCD;


TEST(CoroutineBindingTest1)
{
	ScriptVM vm;

	runScriptFile(vm, "CoroutineTest1.nut");

	CHECK(true);
}

namespace script {
}	// namespace script

TEST(CoroutineBindingTest2)
{
	ScriptVM vm;

	runScriptFile(vm, "CoroutineTest1.nut");

	CHECK(true);
}
