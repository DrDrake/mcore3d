#ifndef __MCD_CORE_BINDING_VMCORE__
#define __MCD_CORE_BINDING_VMCORE__

#include "../ShareLib.h"
#include "../../../3Party/squirrel/squirrel.h"

namespace MCD {
namespace Binding {

class ClassesManager;

class MCD_CORE_API VMCore
{
public:
	explicit VMCore(int initialStackSize=256);

	~VMCore();

	enum State
	{
		OPENING,
		ACTIVE,
		CLOSING
	};

	State state() const { return mState; }

	HSQUIRRELVM getVM() const;

	sal_checkreturn bool runScript(const char* script, bool retVal=false, const char* scriptName="unnamed");

	void collectGarbage();

	static sal_checkreturn bool runScript(HSQUIRRELVM v, const char* script, bool retVal=false, const char* scriptName="unnamed");

private:
	State mState;
	HSQUIRRELVM mSqvm;
	HSQOBJECT mClassesTable;

	friend class ClassesManager;
};	// VMCore

// A handy function to simply execute a script
MCD_CORE_API bool runScript(HSQUIRRELVM vm, const char* script);

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_VMCORE__
