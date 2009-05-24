#ifndef __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
#define __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__

#include "../ShareLib.h"

namespace MCD {

/*	A kind of handle that 
	\warn Be aware of threading problems.
 */
class MCD_CORE_API ScriptOwnershipHandle
{
public:
	ScriptOwnershipHandle();

	~ScriptOwnershipHandle();

	void destroy();

	/*!	Associate a weak pointer pointing to the object at index.
		If the supplied HSQUIRRELVM is null, then any previous
		associated handle will be removed.
	 */
	void setHandle(void* vm, int index);

	bool pushHandle(void* vm);

	void* vm;	// HSQUIRRELVM
	char weakRef[sizeof(void*) * 2];	// HSQOBJECT
};	// ScriptOwnershipHandle

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_SCRIPTOWNERSHIPHANDLE__
