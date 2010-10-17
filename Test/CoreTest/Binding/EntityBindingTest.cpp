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

	CHECK(vm.runScript(
		"local e1 = Entity(\"e1\");\n"
		"local e2 = Entity();\n"
		"e2.asChildOf(e1);\n"
		"local e3 = e2.parent;\n"
		"assert(e1 == e3);\n"
		"e2.destroyThis();\n"
		"assert(!e2.parent);\n"
	));
}
