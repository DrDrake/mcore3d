#include "Pch.h"
#include "ScriptComponent.h"
#include "VMCore.h"
#include "../Entity/Entity.h"
#include "../../../3Party/squirrel/squirrel.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

namespace MCD {
namespace Binding {

ScriptComponent::ScriptComponent()
	: mThreadVM(nullptr)
	, mSuspended(false)
{
}

void ScriptComponent::update(float dt)
{
	if(mSuspended) return;

	HSQUIRRELVM orgVm = reinterpret_cast<HSQUIRRELVM>(scriptVm);
	HSQOBJECT& h = *reinterpret_cast<HSQOBJECT*>(scriptHandle);
	if(!orgVm) return;

	VMCore* core = reinterpret_cast<VMCore*>(sq_getforeignptr(orgVm));
	MCD_ASSUME(core);

	const SQInteger orgOldTop = sq_gettop(orgVm);
	HSQUIRRELVM v = core->allocateThraed();
	MCD_ASSUME(v);

	// Push this ScriptComponent from the original VM to the thread VM
	sq_pushobject(orgVm, h);
	sq_move(v, orgVm, -1);
	sq_pushstring(v, "update" , -1);
	sq_get(v, -2);
	/// stack: scriptComponent, update()

	sq_pushobject(v, h);
	sq_pushfloat(v, dt);
	/// stack: scriptComponent, update(), scriptComponent, dt

	if(SQ_FAILED(sq_call(v, 2, false, true)))
		VMCore::printError(v);
	/// stack: scriptComponent, update()

	if(SQ_VMSTATE_SUSPENDED == sq_getvmstate(v)) {
		mSuspended = true;
		v = nullptr;
	}
	else {
		core->releaseThread(v);
		mSuspended = false;
	}

	sq_settop(orgVm, orgOldTop);
}

ScriptManagerComponent::ScriptManagerComponent(VMCore* vmcore) : mVMCore(vmcore) {}

void ScriptManagerComponent::end(float dt)
{
	MCD_ASSUME(mVMCore);

	while(true)
	{
		void* userData = nullptr;
		const float time = mVMCore->currentTime();	// Putting this line in the loop make a better response

		if(HSQUIRRELVM v = mVMCore->popScheduled(time, &userData)) {
			MCD_ASSUME(userData);
			if(SQ_FAILED(sq_wakeupvm(v, false, false, true, false)))
				VMCore::printError(v);

			if(SQ_VMSTATE_SUSPENDED != sq_getvmstate(v)) {
				mVMCore->releaseThread(v);
				reinterpret_cast<ScriptComponent*>(userData)->mSuspended = false;
			}
		} else
			break;
	}
}

}	// namespace Binding
}	// namespace MCD
