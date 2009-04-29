#ifndef __SCRIPT_VM_CORE___
#define __SCRIPT_VM_CORE___

#include "Base.h"
#include "detail/Language.h"

//
// Forwards
//
namespace script {
namespace detail {
	class ClassesManager;
}	//namspace detail
}	//namespace script

namespace script {

class VMCore: public base::Object
{
public:
	explicit VMCore(int initialStackSize);
	virtual ~VMCore();

	enum State
	{
		vmsOPENING,
		vmsACTIVE,
		vmsCLOSING
	};

	inline State state() const { return _state; }

	HSQUIRRELVM getVM() const;

	void collectGarbage();

private:
	State       _state;
	HSQUIRRELVM _vm;
	HSQOBJECT   _classesTable;

//friends:
	friend class detail::ClassesManager;
};

// A handy function to simply execute a script
extern bool runScript(HSQUIRRELVM vm, const xchar* script);

}	//namespace script

#endif//__SCRIPT_VM_CORE___
