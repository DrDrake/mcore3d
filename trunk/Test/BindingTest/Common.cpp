#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"
#include <fstream>

bool runScriptFile(MCD::ScriptVM& vm, const char* filePath)
{
	std::ifstream file(filePath);
	if(file) {
		std::ostringstream buffer;
		buffer << file.rdbuf();
		file.close();

		return vm.runScript(buffer.str().c_str());
	}
	else
		std::cerr << "File '" << filePath << "' not found\n";

	return false;
}
