#include "Pch.h"
#include "../../../MCD/Core/Binding/CoreBindings.h"
#include "../../../MCD/Core/Binding/VMCore.h"
#include "../../../MCD/Core/System/RawFileSystem.h"
#include <fstream>

using namespace MCD;

TEST(ScriptWithIOStream_BindingTest)
{
	Binding::VMCore vm;

	{	// Run from string source
		std::ifstream is("Binding/squnit.nut");
		CHECK(vm.runScript(is));
	}

	{	// Save byte code
		std::ifstream is("Binding/squnit.nut");
		std::ofstream os("Binding/squnit.bnut", std::ios::binary);
		CHECK(vm.loadScript(is));
		CHECK(vm.saveByteCode(os));
	}

	{	// Run the byte code
		std::ifstream is("Binding/squnit.bnut", std::ios::binary);
		CHECK(vm.runScript(is));
		is.close();

		RawFileSystem fs("");
		fs.remove("Binding/squnit.bnut");
	}

	{	// Non existing source
		std::ifstream is("", std::ios::binary);
		CHECK(!vm.runScript(is));
	}
}

TEST(StrLen_BindingTest)
{
	Binding::VMCore vm;
	CHECK(vm.runScript("local a=0;~!@#$%", 10));
}
