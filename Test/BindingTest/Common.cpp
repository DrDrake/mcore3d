#include "Pch.h"
#include "Common.h"
#include "../../Binding/Binding.h"
#include <fstream>

void runScriptFile(MCD::ScriptVM& vm, const char* filePath)
{
	std::wifstream file(filePath);
	std::wstring data;
	if(file) {
		std::wostringstream buffer;
		buffer << file.rdbuf();
		file.close();

		vm.runScript(buffer.str().c_str());
	}
	else
		std::cerr << "File '" << filePath << "' not found\n";
}