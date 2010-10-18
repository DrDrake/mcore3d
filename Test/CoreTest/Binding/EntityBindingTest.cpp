#include "Pch.h"
#include "../../../MCD/Core/Binding/CoreBindings.h"
#include "../../../MCD/Core/Binding/VMCore.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include <fstream>

using namespace MCD;

TEST(Entity_BindingTest)
{
	Binding::VMCore vm;
	Binding::registerCoreBinding(vm);

	{	std::ifstream is("Binding/squnit.nut");
		vm.runScript(is);
	}

	{	std::ifstream is("Binding/EntityTest.nut");
		vm.runScript(is);
	}
}
