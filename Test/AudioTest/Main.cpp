#include "Pch.h"
#include "../../MCD/Core/System/Platform.h"

#include <fstream>

int main(int, char const*[])
{
#ifdef MCD_VC
	// Tell the c-run time to do memory check at program shut down
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	size_t ret;
	CppTestHarness::TestRunner runner;
	runner.ShowTestName(true);

	// RunTest.txt will not be committed
	// Just creates a text file named "RunTest.txt" in "{mcdRoot}\Test\RenderTest"
	// ; and type in which test to run (only one test is supported)
	std::ifstream infile;

	infile.open("RunTest.txt", std::ifstream::in);
	if(infile.good())
	{
		// we found it, read the test2Run and close the file
		std::string test2Run;
		infile >> test2Run;
		infile.close();

		ret = runner.RunTest(test2Run.c_str());
	}
	else
	{
		// "RunTest.txt" not found, just run all testes :-)
		ret = runner.RunAllTests();
	}

	return int(ret);
}
