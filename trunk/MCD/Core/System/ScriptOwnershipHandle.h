#ifndef __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
#define __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__

#include "../ShareLib.h"

namespace MCD {

// TODO: Handle multi-thread problem

/*	A handle that bring strong linkage between a cpp and script object.
	It work together with jkbind's addHandleToObject() and pushHandleFromObject().
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

	/*!	Associate a weak pointer pointing to the object at index.
		\note If the supplied HSQUIRRELVM is null, then any previous
			associated handle will be removed.
		\note If this function is called a second time with the parameter
			\em vm differ as before, then the function simply returns.
	 */
	void setHandle(void* vm, int index);

	bool pushHandle(void* vm);

	void* vm;	//!< The scripting virtual machine of the type HSQUIRRELVM

	/*!	We use a char buffer to represent a HSQOBJECT object,
		to erase the dependency of squirrel headers. Static
		assert is performed on the cpp to assert the buffer
		size is always valid.
	 */
	char weakRef[sizeof(void*) * 2];
};	// ScriptOwnershipHandle

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
