#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Core/Binding/VMCore.h"
#include <fstream>

using namespace MCD;

TEST(Many_RenderBindingTest)
{
	Framework framework;

	std::ifstream is("RenderBindingTest.nut");
	framework.vm().runScript(is);

	framework.mainLoop();
}

TEST(EarthTest)
{
	Framework framework;

	std::ifstream is("EarthTest.nut");
	framework.vm().runScript(is);

	framework.mainLoop();
}
