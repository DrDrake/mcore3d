#ifndef __MCD_CORE_BINDING_SCRIPTOBJECT__
#define __MCD_CORE_BINDING_SCRIPTOBJECT__

#include "../ShareLib.h"
#include "../System/Platform.h"
#include "../../../3Party/squirrel/squirrel.h"

namespace MCD {
namespace Binding {

class MCD_CORE_API ScriptObject
{
public:
	explicit ScriptObject(HSQUIRRELVM vm);

	ScriptObject(const ScriptObject& o);

	ScriptObject(HSQUIRRELVM vm, HSQOBJECT o);

	~ScriptObject() { sq_release(mSqvm, &mObj); }

	ScriptObject& operator=(const ScriptObject& o);

// Attributes
	HSQUIRRELVM getHostVirtualMachine() const { return mSqvm; }

	HSQOBJECT& handle() { return mObj; }

	const HSQOBJECT& handle() const { return mObj; }

	SQObjectType type() const { return mObj._type; }

// Operations
	sal_checkreturn bool getFromStack(int idx);

	/// Release (any) reference and reset mObj.
	void reset();

// Userdata handling:
	bool setInstanceUp(SQUserPointer up);
	SQUserPointer getInstanceUp(SQUserPointer tag) const;

// Type and value query:
	bool isNull() const { return sq_isnull(mObj); }
	int isNumeric() const { return sq_isnumeric(mObj); }

	SQBool toBool() const { return sq_objtobool((HSQOBJECT*)&mObj); }
	SQInteger toInteger() const { return sq_objtointeger((HSQOBJECT*)&mObj); }
	SQFloat toFloat() const { return sq_objtofloat((HSQOBJECT*)&mObj); }
	sal_maybenull const char* toString() const { return sq_objtostring((HSQOBJECT*)&mObj); }

	bool getTypeTag(SQUserPointer* typeTag) const;

// Table operations:
	ScriptObject getValue(const char* key) const;

	sal_checkreturn bool exists(const char* key) const;
	sal_checkreturn bool setValue(const char* key, const ScriptObject& val);
	sal_checkreturn bool setValue(const char* key, SQInteger n);

private:
	sal_checkreturn bool _getSlot(const char* name) const;

	HSQOBJECT mObj;
	HSQUIRRELVM mSqvm;
};	// ScriptObject

void push(HSQUIRRELVM v, ScriptObject& value);

}	// nemspace Binding
}	//namespace MCD

#endif	// __MCD_CORE_BINDING_SCRIPTOBJECT__
