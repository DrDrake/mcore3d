#include "Pch.h"
#include "../../MCD/Binding/Binding.h"
#include "../../MCD/Binding/ScriptComponentManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/RawFileSystem.h"

using namespace MCD;

TEST(ScriptComponentTest)
{
	RawFileSystem fs("");
	ScriptVM vm;
	ScriptComponentManager mgr;
	CHECK(mgr.init(vm, fs));
	Entity root;
	mgr.registerRootEntity(root);

	CHECK(mgr.doFile("ScriptComponentTestInit.nut", true));
}

TEST(CreateEntityFromScripTest)
{
	RawFileSystem fs("");
	ScriptVM vm;
	ScriptComponentManager mgr;
	CHECK(mgr.init(vm, fs));

	{	// C++ keep ownership
		Entity* e = mgr.runScripAsEntity("local e=Entity(\"CreateEntityFromScripTest\");return e;");
		CHECK(e);
		if(e) {
			CHECK_EQUAL("CreateEntityFromScripTest", e->name);
			delete e;
		}
	}

	{	// Script keep ownership
		Entity* e = mgr.runScripAsEntity("local e=Entity(\"CreateEntityFromScripTest\");return e;", true);
		CHECK(e);
		if(e)
			CHECK_EQUAL("CreateEntityFromScripTest", e->name);
	}

	{	// Syntax error
//		Entity* e = mgr.runScripAsEntity("abcdefg");
//		CHECK(!e);
	}

	{	// The script does not returning anything
		Entity* e = mgr.runScripAsEntity("local a=0;");
		CHECK(!e);
	}
}
