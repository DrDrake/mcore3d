#include "Pch.h"
#include "ScriptOwnershipHandle.h"
#include "Platform.h"
#include "StaticAssert.h"
#include "../../../3Party/squirrel/squirrel.h"
#include "../../../3Party/squirrel/squirrel/sqclass.h"
#include <assert.h>

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "squirrel")
#	endif
#endif

namespace MCD {

// NOTE: The mRef variable is not initialized, we use vm
// to indicate the whole ScriptOwnershipHandle is in a valid
// state or not.
ScriptOwnershipHandle::ScriptOwnershipHandle()
	: vm(nullptr)
{
	MCD_STATIC_ASSERT(sizeof(mRef) == sizeof(HSQOBJECT));
}

ScriptOwnershipHandle::~ScriptOwnershipHandle() {
	destroy();
}

void ScriptOwnershipHandle::destroy()
{
	if(!vm)
		return;

	HSQOBJECT* ref = reinterpret_cast<HSQOBJECT*>(mRef);
	MCD_ASSUME(ref);

	SQObject* o = nullptr;
	if(ref->_type == OT_INSTANCE)
		o = ref;
	else if(ref->_type == OT_WEAKREF) {
		// Get the SQObject of what the weak reference pointing to.
		o = &(ref->_unVal.pWeakRef->_obj);
	}
	else if(ref->_type == OT_NULL) {
		vm = nullptr;
		return;
	}
	else { MCD_ASSERT(false); }
	MCD_ASSUME(o);

	// The SQObject may be null, if there are no more reference to this object
	// in the VM. That's also the reason why we need to store SQObject via weak
	// reference but not the SQObject itself.
	if(sq_isinstance(*o)) {
		// Destroy (in squirrel api it's call Finalize) the corresponding handle
		// of this object.
		o->_unVal.pInstance->Finalize();
	}

	// Release the WEAK reference
	if(ref->_type == OT_WEAKREF)
		sq_release(reinterpret_cast<HSQUIRRELVM>(vm), ref);

	vm = nullptr;
}

void ScriptOwnershipHandle::setHandle(void* v, int index, bool keepStrongRef)
{
	if(!v) return;
	if(vm) return;	// Ignore setting the handle from secondary VM

	vm = v;

	HSQOBJECT* ref = reinterpret_cast<HSQOBJECT*>(mRef);
	HSQUIRRELVM& _vm = reinterpret_cast<HSQUIRRELVM&>(vm);
	MCD_ASSUME(ref);

	if(keepStrongRef) {
		sq_getstackobj(_vm, index, ref);
		sq_addref(_vm, ref);
	}
	else {
		sq_weakref(_vm, index);	// Create a weak reference to the object at index
		sq_getstackobj(_vm, -1, ref);
		sq_addref(_vm, ref);
		sq_pop(_vm, 1);
	}
}

void ScriptOwnershipHandle::useStrongReference(bool strong)
{
	if(!vm) return;

	HSQOBJECT* ref = reinterpret_cast<HSQOBJECT*>(mRef);
	HSQUIRRELVM& _vm = reinterpret_cast<HSQUIRRELVM&>(vm);
	MCD_ASSUME(ref);

	if(strong && ref->_type == OT_WEAKREF) {
		// Transform the weak ref to strong ref
		HSQOBJECT o = ref->_unVal.pWeakRef->_obj;
		sq_addref(_vm, &o);
		sq_release(_vm, ref);
		*ref = o;
	} else if(!strong && ref->_type == OT_INSTANCE) {
		// Transform the strong ref to weak ref
		sq_pushobject(_vm, *ref);
		sq_release(_vm, ref);
		sq_weakref(_vm, -1);
		sq_getstackobj(_vm, -1, ref);
		sq_addref(_vm, ref);
		sq_pop(_vm, 2);
	}
}

bool ScriptOwnershipHandle::pushHandle(void* v)
{
	// Ok, even v != vm, there is no problem.
	if(!v) return false;

	HSQOBJECT* ref = reinterpret_cast<HSQOBJECT*>(mRef);
	MCD_ASSUME(ref);

	SQObject* o = nullptr;
	if(ref->_type == OT_INSTANCE || ref->_type == OT_NULL)
		o = ref;
	else if(ref->_type == OT_WEAKREF)
	{
		// Get the SQObject of what the weak reference pointing to.
		o = &(ref->_unVal.pWeakRef->_obj);	

		// The strong reference may already released
		if(!sq_isinstance(*o)) {
			sq_release(reinterpret_cast<HSQUIRRELVM>(v), ref);	// TODO: Is passing "v" as the VM correct?
			sq_resetobject(ref);
			vm = nullptr;
			return false;
		}
	}
	else { MCD_ASSERT(false); }
	MCD_ASSUME(o);

	sq_pushobject(reinterpret_cast<HSQUIRRELVM>(v), *o);	// NOTE: We are passing v but not vm.
	return true;
}

}	// namespace MCD
