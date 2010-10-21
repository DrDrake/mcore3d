#include "Pch.h"
#include "ScriptComponent.h"
#include "VMCore.h"
#include "../Entity/Entity.h"
#include "../../../3Party/squirrel/squirrel.h"

namespace MCD {
namespace Binding {

void ScriptComponent::update(float dt)
{
	HSQUIRRELVM v = reinterpret_cast<HSQUIRRELVM>(scriptVm);
	HSQOBJECT& h = *reinterpret_cast<HSQOBJECT*>(scriptHandle);
	if(!v) return;

	const int oldTop = sq_gettop(v);

	sq_pushobject(v, h);
	sq_pushstring(v, "update" ,-1);
	sq_get(v, -2);
	/// stack: scriptComponent, update()

	sq_pushobject(v, h);
	sq_pushfloat(v, dt);
	/// stack: scriptComponent, update(), scriptComponent, dt

	if(SQ_FAILED(sq_call(v, 2, false, true)))
		VMCore::printError(v);
	/// stack: scriptComponent, update()

	sq_settop(v, oldTop);
}

}	// namespace Binding
}	// namespace MCD
