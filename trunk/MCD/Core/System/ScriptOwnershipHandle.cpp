#include "Pch.h"
#include "ScriptOwnershipHandle.h"
#include "Platform.h"
#include "StaticAssert.h"
#include <assert.h>
#include "../../../3Party/squirrel/squirrel.h"
#include "../../../3Party/squirrel/squirrel/sqclass.h"

#ifndef NDEBUG
#	pragma comment(lib, "squirreld")
#else
#	pragma comment(lib, "squirrel")
#endif

namespace MCD {

// NOTE: The weakRef variable is not initialized, we use vm
// to indicate the whole ScriptOwnershipHandle is in a valid
// state or not.
ScriptOwnershipHandle::ScriptOwnershipHandle()
	: vm(nullptr)
{
	MCD_STATIC_ASSERT(sizeof(weakRef) == sizeof(HSQOBJECT));
}

ScriptOwnershipHandle::~ScriptOwnershipHandle() {
	destroy();
}

void ScriptOwnershipHandle::destroy()
{
	if(!vm)
		return;

	HSQOBJECT* _weakRef = reinterpret_cast<HSQOBJECT*>(weakRef);

	// Get the SQObject of what the weak reference pointing to.
	SQObject& o = _weakRef->_unVal.pWeakRef->_obj;

	// The SQObject may be null, if there are no more reference to this object
	// in the VM. That's also the reason why we need to store SQObject via weak
	// reference but not the SQObject itself.
	if(sq_isinstance(o)) {
		// Destroy (in squirrel api it's call Finalize) the corresponding handle
		// of this object.
		o._unVal.pInstance->Finalize();
	}

	// Release the WEAK reference
	sq_release(reinterpret_cast<HSQUIRRELVM>(vm), _weakRef);

	vm = nullptr;
}

void ScriptOwnershipHandle::setHandle(void* v, int index)
{
	HSQOBJECT* _weakRef = reinterpret_cast<HSQOBJECT*>(weakRef);
	HSQUIRRELVM& _vm = reinterpret_cast<HSQUIRRELVM&>(vm);

	if(vm)	// Remove previous reference
		sq_release(_vm, _weakRef);
	vm = v;

	if(!v)
		return;

	sq_weakref(_vm, index);	// Create a weak reference to the object at index
	sq_getstackobj(_vm, -1, _weakRef);
	sq_addref(_vm, _weakRef);
	sq_pop(_vm, 1);
}

bool ScriptOwnershipHandle::pushHandle(void* v)
{
	// A safty check in multiple VM situation
	if(!v || v != vm)
		return false;

	HSQOBJECT* _weakRef = reinterpret_cast<HSQOBJECT*>(weakRef);

	// Get the SQObject of what the weak reference pointing to.
	SQObject& o = _weakRef->_unVal.pWeakRef->_obj;
	if(!sq_isinstance(o))	// The strong reference may already released
		return false;

	sq_pushobject(reinterpret_cast<HSQUIRRELVM>(vm), o);
	return true;
}

}	// namespace MCD
