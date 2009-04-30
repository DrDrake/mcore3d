#include "Pch.h"
#include "../../Binding/Binding.h"
#include <fstream>

using namespace MCD;

TEST(Entity_BindingTest)
{
	ScriptVM vm;

	std::wifstream file("EntityTest.nut");
	std::wstring data;
	if(file) {
		std::wostringstream buffer;
		buffer << file.rdbuf();
		file.close();

		data = buffer.str();
	}

	vm.runScript(data.c_str());

	CHECK(true);
}
