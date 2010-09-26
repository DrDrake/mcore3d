#include "Pch.h"
#include "ScriptObject.h"

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

namespace MCD {

namespace Binding {

ScriptObject::ScriptObject(HSQUIRRELVM vm)
	: mSqvm(vm)
{
	sq_resetobject(&mObj);
}

ScriptObject::ScriptObject(const ScriptObject& o)
	: mSqvm(o.mSqvm)
	, mObj(o.mObj)
{
	sq_addref(mSqvm, &mObj);
}

ScriptObject::ScriptObject(HSQUIRRELVM vm, HSQOBJECT o)
	: mSqvm(vm)
	, mObj(o)
{
	sq_addref(mSqvm, &mObj);
}

ScriptObject& ScriptObject::operator=(const ScriptObject& o)
{
	MCD_ASSERT(mSqvm == o.mSqvm);
	HSQOBJECT t;
	t = o.mObj;
	sq_addref(mSqvm, &t);
	sq_release(mSqvm, &mObj);
	mObj = t;
	return *this;
}

bool ScriptObject::getFromStack(int idx)
{
	HSQOBJECT t;
	if(SQ_SUCCEEDED(sq_getstackobj(mSqvm, idx, &t))) {
		sq_addref(mSqvm, &t);
		sq_release(mSqvm, &mObj);
		mObj = t;
		return true;
	}
	return false;
}

void ScriptObject::reset(void)
{
	sq_release(mSqvm, &mObj);
	sq_resetobject(&mObj);
}

bool ScriptObject::setInstanceUp(SQUserPointer up)
{
	MCD_ASSERT(sq_isinstance(mObj));
	sq_pushobject(mSqvm, mObj);
	sq_setinstanceup(mSqvm, -1, up);
	sq_poptop(mSqvm);
	return true;
}

SQUserPointer ScriptObject::getInstanceUp(SQUserPointer tag) const
{
	SQUserPointer up = 0;
	sq_pushobject(mSqvm, mObj);
	sq_getinstanceup(mSqvm, -1, (SQUserPointer*)&up, tag);
	sq_poptop(mSqvm);
	return up;
}

bool ScriptObject::getTypeTag(SQUserPointer* typeTag) const
{
	return SQ_SUCCEEDED(sq_getobjtypetag((HSQOBJECT*)&mObj, typeTag));
}

bool ScriptObject::exists(const char* key) const
{
	bool ret = _getSlot(key);
	sq_poptop(mSqvm);
	return ret;
}

ScriptObject ScriptObject::getValue(const char* key) const
{
	ScriptObject ret(mSqvm);
	if(_getSlot(key))
		MCD_VERIFY(ret.getFromStack(-1));
	sq_poptop(mSqvm);
	return ret;
}

#define _SETVALUE_STR_BEGIN \
bool ret = false; \
int top = sq_gettop(mSqvm); \
sq_pushobject(mSqvm, mObj); \
sq_pushstring(mSqvm, key, -1);

#define _SETVALUE_STR_END \
if(SQ_SUCCEEDED(sq_rawset(mSqvm, -3))) { \
	ret = true; \
} \
sq_settop(mSqvm,top); \
return ret;

bool ScriptObject::setValue(const char* key, const ScriptObject& val)
{
	_SETVALUE_STR_BEGIN
	sq_pushobject(mSqvm, val.mObj);
	_SETVALUE_STR_END
}

bool ScriptObject::setValue(const char* key, SQInteger n)
{
	_SETVALUE_STR_BEGIN
	sq_pushinteger(mSqvm, n);
	_SETVALUE_STR_END
}

#undef _SETVALUE_STR_BEGIN
#undef _SETVALUE_STR_END

bool ScriptObject::_getSlot(const char* name) const
{
	sq_pushobject(mSqvm, mObj);
	sq_pushstring(mSqvm, name, -1);
	return SQ_SUCCEEDED(sq_get(mSqvm, -2));
}

void push(HSQUIRRELVM mSqvm, ScriptObject& value)
{
	sq_pushobject(mSqvm, value.handle());
}

}	// namespace Binding

}	// namespace MCD
