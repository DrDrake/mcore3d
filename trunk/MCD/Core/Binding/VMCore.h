#ifndef __MCD_CORE_BINDING_VMCORE__
#define __MCD_CORE_BINDING_VMCORE__

#include "../ShareLib.h"
#include "../../../3Party/squirrel/squirrel.h"
//#include "Base.h"
//#include "detail/Language.h"

namespace MCD {
namespace Binding {

class ClassesManager;

class MCD_CORE_API VMCore//: public base::Object
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

	bool runScript(const char* script, bool retVal=false, const char* scriptName="unnamed");

	void collectGarbage();

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
