#include "Pch.h"
#include "../../MCD/Binding/ScriptComponentManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/RawFileSystem.h"

using namespace MCD;

TEST(ScriptComponentTest)
{
	RawFileSystem fs(L"");
	ScriptComponentManager mgr(fs);
	Entity root;
	mgr.registerRootEntity(root);

	CHECK(mgr.doFile(L"ScriptComponentTestInit.nut", true));
}
