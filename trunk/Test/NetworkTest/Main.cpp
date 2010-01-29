#include "Pch.h"
#include "../../MCD/Core/System/Platform.h"

int main(int, char const*[])
{
#ifdef MCD_VC
	// Tell the c-run time to do memory check at program shut down
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	size_t ret;
	CppTestHarness::TestRunner runner;
	runner.ShowTestName(false);

//	ret = runner.RunAllTests();
	ret = runner.RunTest("Entity_BindingTest");

	return int(ret);
}
