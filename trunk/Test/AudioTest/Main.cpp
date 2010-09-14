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

	{	// Copy render API specific dll into MCDRender(d).dll
		using namespace std;
		// NOTE: The "SelectDll.txt" should contains multiple pairs of text:
		// ../../Bin/MCDRenderDX9.dll
		// ../../Bin/MCDRender.dll
		// ../../Bin/MCDRenderDX9d.dll
		// ../../Bin/MCDRenderd.dll
		ifstream infile("SelectDll.txt", ifstream::in);
		string srcDll, destDll;

		while(infile.good())
		{
			infile >> srcDll >> destDll;
			printf("Replacing DLL \"%s\" with \"%s\"... ", destDll.c_str(), srcDll.c_str());

			// Reference: http://blog.emptycrate.com/node/264
			ifstream f1(srcDll.c_str(), fstream::binary);
			ofstream f2(destDll.c_str(), fstream::trunc|fstream::binary);
			if(f1.good() && f2.good()) {
				f2 << f1.rdbuf();
				printf("Succeed\n");
			}
			else {
				if(!f1.good())
					printf("Failed to open file %s\n", srcDll.c_str());
				if(!f2.good())
					printf("Failed to open file %s\n", destDll.c_str());
			}
		}
	}

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
