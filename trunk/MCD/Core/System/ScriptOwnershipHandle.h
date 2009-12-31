#ifndef __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
#define __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__

#include "../ShareLib.h"

namespace MCD {

// TODO: Handle multi-thread problem

/*	A handle that bring strong linkage between a cpp and script object.
	It work together with jkbind's addHandleToObject() and pushHandleFromObject().

	Owning this ScriptOwnershipHandle also means having the ultimate strong ownership
	of it's pointee object; because once destroy() is called, even there are still many
	strong reference on the script VM side, those reference will force to null too.

	\warn Be aware of threading problems, especially in the destructor
 */
class MCD_CORE_API ScriptOwnershipHandle
{
public:
	ScriptOwnershipHandle();

	//! Destructor will invoke destroy.
	~ScriptOwnershipHandle();

	/*!	Inform the Script VM that this object is gonna destroy.
		All reference to this object in the script should become null afterward.
		\note This function is invoked in ~ScriptOwnershipHandle() automatically,
			so normally user need not to invoke this function.
	 */
	void destroy();

	/*!	Only set the ScriptOwnershipHandle to it's initial null state,
		without forcing to destroy the object. Also aim to release the very strong
		ownership.
	 */
	void setNull();

	/*!	Prevents the ScriptOwnershipHandle calling the pointee's C++ destructor.
		It's usefull when actually your pointee own this ScriptOwnershipHandle,
		call removeReleaseHook() in the destructor can prevent recursive destructor call.
	 */
	void removeReleaseHook();

	/*!	Point the ScriptOwnershipHandle to the squirrel object at index.
		\param keepStrongRef Set to true if you want ScriptOwnershipHandle hold
			a strong reference to the squirrel object. That is, the squirrel object
			will absolutely follow the life time of ScriptOwnershipHandle.
		\note Do nothing if the \em vm parameter is null.
		\note Only the first time invocation is effective, all subsequence call will
			be simply ignored.
	 */
	void setHandle(void* vm, int index, bool keepStrongRef=false);

	//!	Change to use strong or weak reference after the initial choice passed in setHandle().
	void useStrongReference(bool strong);

	//! Push the stored squirrel object handle to the VM stack.
	bool pushHandle(void* vm);

	/*!	Clone the current script object, and returns it's corresponding C++ part if success.
		\param other Another ScriptOwnershipHandle to temporary hold a strong reference to
			the returning object. It must be in a null state when passing in.
	 */
	sal_maybenull void* cloneTo(ScriptOwnershipHandle& other) const;

	void* vm;	//!< The scripting virtual machine of the type HSQUIRRELVM

	/*!	We use a char buffer to represent a HSQOBJECT object,
		to erase the dependency of squirrel headers. Static
		assert is performed on the cpp to assert the buffer
		size is always valid.
	 */
	char mRef[sizeof(void*) * 2];
};	// ScriptOwnershipHandle

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
