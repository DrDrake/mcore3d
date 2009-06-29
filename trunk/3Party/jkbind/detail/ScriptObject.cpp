#include "ScriptObject.h"

namespace script {
namespace detail {

ScriptObject& ScriptObject::operator =(const ScriptObject& o)
{
	jkSCRIPT_LOGIC_ASSERT(_vm == o._vm);
	HSQOBJECT t;
	t = o._o;
	sq_addref(_vm, &t);
	sq_release(_vm, &_o);
	_o = t;
	return *this;
}

ScriptObject& ScriptObject::operator =(int n)
{
	sq_pushinteger(_vm, n);
	attachToStackObject(-1);
	sq_pop(_vm, 1);
	return *this;
}

void ScriptObject::attachToStackObject(int idx)
{
	HSQOBJECT t;
	sq_getstackobj(_vm, idx, &t);
	sq_addref(_vm, &t);
	sq_release(_vm, &_o);
	_o = t;
}

void ScriptObject::reset(void)
{
	sq_release(_vm, &_o);
	sq_resetobject(&_o);
}

ScriptObject ScriptObject::clone()
{
	ScriptObject ret(_vm);
	if(getType() == OT_TABLE || getType() == OT_ARRAY)
	{
		sq_pushobject(_vm, _o);
		sq_clone(_vm, -1);
		ret.attachToStackObject(-1);
		sq_pop(_vm, 2);
	}
	return ret;
}

bool ScriptObject::arrayResize(SQInteger newSize)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
	sq_pushobject(_vm, getObjectHandle());
	bool res = sq_arrayresize(_vm, -1, newSize) == SQ_OK;
	sq_pop(_vm, 1);
	return res;
}

bool ScriptObject::arrayExtend(SQInteger amount)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
	SQInteger newLen = length() + amount;
	return arrayResize(newLen);
}

bool ScriptObject::arrayReverse(void)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
	sq_pushobject(_vm, getObjectHandle());
	bool res = sq_arrayreverse(_vm, -1) == SQ_OK;
	sq_pop(_vm, 1);
	return res;
}

ScriptObject ScriptObject::arrayPop(SQBool returnPoppedVal)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
	ScriptObject ret(_vm);
	int top = sq_gettop(_vm);
	sq_pushobject(_vm, getObjectHandle());
	if (sq_arraypop(_vm, -1, returnPoppedVal) == SQ_OK) {
		if (returnPoppedVal) {
			ret.attachToStackObject(-1);
		}
	}
	sq_settop(_vm, top);
	return ret;
}

void ScriptObject::arrayAppend(const ScriptObject& o)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
	sq_pushobject(_vm, _o);
	sq_pushobject(_vm, o._o);
	sq_arrayappend(_vm, -2);
	sq_pop(_vm, 1);
}


int ScriptObject::length() const
{
	jkSCRIPT_LOGIC_ASSERT((sq_isarray(_o) || sq_istable(_o) || sq_isstring(_o)));
	sq_pushobject(_vm, _o);
	int ret = sq_getsize(_vm, -1);
	sq_pop(_vm, 1);
	return ret;
}

bool ScriptObject::setInstanceUp(SQUserPointer up)
{
	jkSCRIPT_LOGIC_ASSERT(sq_isinstance(_o));
	sq_pushobject(_vm, _o);
	sq_setinstanceup(_vm, -1, up);
	sq_pop(_vm, 1);
	return true;
}

SQUserPointer ScriptObject::getInstanceUp(SQUserPointer tag) const
{
	SQUserPointer up = 0;
	sq_pushobject(_vm, _o);
	sq_getinstanceup(_vm, -1, (SQUserPointer*)&up, tag);
	sq_pop(_vm, 1);
	return up;
}

bool ScriptObject::setDelegate(ScriptObject &obj)
{
	jkSCRIPT_LOGIC_ASSERT((obj.getType() == OT_TABLE) || (obj.getType() == OT_NULL));
	jkSCRIPT_LOGIC_ASSERT((_o._type == OT_USERDATA) || (_o._type == OT_TABLE));
	sq_pushobject(_vm, _o);
	sq_pushobject(_vm, obj._o);
	jkSCRIPT_API_VERIFY(sq_setdelegate(_vm, -2));
	return true;
}

ScriptObject ScriptObject::getDelegate()
{
	jkSCRIPT_LOGIC_ASSERT(_o._type == OT_TABLE || _o._type == OT_USERDATA);
	ScriptObject ret(_vm);
	int top = sq_gettop(_vm);
	sq_pushobject(_vm, _o);
	sq_getdelegate(_vm, -1);
	ret.attachToStackObject(-1);
	sq_settop(_vm, top);
	return ret;
}

bool ScriptObject::getTypeTag(SQUserPointer* typeTag)
{
	if (SQ_SUCCEEDED(sq_getobjtypetag(&_o, typeTag))) {
		return true;
	}
	return false;
}

bool ScriptObject::exists(const SQChar* key) const
{
	bool ret = false;
	if(_getSlot(key)) {
		ret = true;
	}
	sq_pop(_vm, 1);
	return ret;
}

ScriptObject ScriptObject::getValue(const SQChar* key) const
{
	ScriptObject ret(_vm);
	if(_getSlot(key)) {
		ret.attachToStackObject(-1);
		sq_pop(_vm, 1);
	}
	sq_pop(_vm, 1);
	return ret;
}

#define _SETVALUE_STR_BEGIN \
bool ret = false; \
int top = sq_gettop(_vm); \
sq_pushobject(_vm, _o); \
sq_pushstring(_vm, key, -1);

#define _SETVALUE_STR_END \
if(SQ_SUCCEEDED(sq_rawset(_vm, -3))) { \
	ret = true; \
} \
sq_settop(_vm,top); \
return ret;

bool ScriptObject::setValue(const SQChar* key, const ScriptObject& val)
{
	_SETVALUE_STR_BEGIN
	sq_pushobject(_vm, val._o);
	_SETVALUE_STR_END
}

bool ScriptObject::setValue(const SQChar* key, SQInteger n)
{
	_SETVALUE_STR_BEGIN
	sq_pushinteger(_vm, n);
	_SETVALUE_STR_END
}

#undef _SETVALUE_STR_BEGIN
#undef _SETVALUE_STR_END

bool ScriptObject::_getSlot(const xchar* name) const
{
	sq_pushobject(_vm, _o);
	sq_pushstring(_vm, name, -1);
	if(SQ_SUCCEEDED(sq_get(_vm, -2))) {
		return true;
	}

	return false;
}

}   //namespace detail
}   //namespace script
