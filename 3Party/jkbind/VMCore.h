#ifndef __SCRIPT_VM_CORE___
#define __SCRIPT_VM_CORE___

#include "Base.h"
#include "detail/Language.h"

namespace script {

namespace detail {
	class ClassesManager;
}	// namspace detail

class VMCore: public base::Object
{
public:
	JKBIND_API VMCore(int initialStackSize);
	JKBIND_API virtual ~VMCore();

	enum State
	{
		vmsOPENING,
		vmsACTIVE,
		vmsCLOSING
	};

	State state() const { return _state; }

	JKBIND_API HSQUIRRELVM getVM() const;

	JKBIND_API void collectGarbage();

private:
	State       _state;
	HSQUIRRELVM _vm;
	HSQOBJECT   _classesTable;

//friends:
	friend class detail::ClassesManager;
};	// VMCore

// A handy function to simply execute a script
JKBIND_API bool runScript(HSQUIRRELVM vm, const xchar* script);

}	//namespace script

#endif//__SCRIPT_VM_CORE___
