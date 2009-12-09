#include "Pch.h"
#include "../../MCD/Binding/Binding.h"
#include "../../MCD/Binding/ScriptComponentManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/RawFileSystem.h"

using namespace MCD;

TEST(ScriptComponentTest)
{
	RawFileSystem fs(L"");
	ScriptVM vm;
	ScriptComponentManager mgr;
	CHECK(mgr.init(vm, fs));
	Entity root;
	mgr.registerRootEntity(root);

	CHECK(mgr.doFile(L"ScriptComponentTestInit.nut", true));
}

TEST(CreateEntityFromScripTest)
{
	RawFileSystem fs(L"");
	ScriptVM vm;
	ScriptComponentManager mgr;
	CHECK(mgr.init(vm, fs));

	{	// C++ keep ownership
		Entity* e = mgr.runScripAsEntity(L"local e=Entity(\"CreateEntityFromScripTest\");return e;");
		CHECK(e);
		if(e) {
			CHECK_EQUAL(L"CreateEntityFromScripTest", e->name);
			delete e;
		}
	}

	{	// Script keep ownership
		Entity* e = mgr.runScripAsEntity(L"local e=Entity(\"CreateEntityFromScripTest\");return e;", true);
		CHECK(e);
		if(e)
			CHECK_EQUAL(L"CreateEntityFromScripTest", e->name);
	}

	{	// Syntax error
//		Entity* e = mgr.runScripAsEntity(L"abcdefg");
//		CHECK(!e);
	}

	{	// The script does not returning anything
		Entity* e = mgr.runScripAsEntity(L"local a=0;");
		CHECK(!e);
	}
}
