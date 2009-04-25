#ifndef ___SCRIPT_SCRIPTOBJECT___
#define ___SCRIPT_SCRIPTOBJECT___

#include "Checking.h"
#include "Language.h"
#include "../Types.h"

namespace script {
namespace detail {

class ScriptObject
{
public:
	~ScriptObject()
	{
		sq_release(_vm, &_o);
	}

	HSQUIRRELVM getHostVirtualMachine() const
	{
		return _vm;
	}

	ScriptObject(HSQUIRRELVM vm)
		:_vm(vm)
	{
		sq_resetobject(&_o);
	}

	ScriptObject(const ScriptObject& o)
		:_vm(o._vm),
		_o(o._o)
	{
		sq_addref(_vm, &_o);
	}

	ScriptObject(HSQUIRRELVM vm, HSQOBJECT o)
		:_vm(vm),
		_o(o)
	{
		sq_addref(_vm, &_o);
	}

	ScriptObject & operator =(const ScriptObject& o)
	{
		jkSCRIPT_LOGIC_ASSERT(_vm == o._vm);
		HSQOBJECT t;
		t = o._o;
		sq_addref(_vm, &t);
		sq_release(_vm, &_o);
		_o = t;
		return *this;
	}

	ScriptObject& operator =(int n)
	{
		sq_pushinteger(_vm, n);
		attachToStackObject(-1);
		sq_pop(_vm, 1);
		return *this;
	}

	void attachToStackObject(int idx)
	{
		HSQOBJECT t;
		sq_getstackobj(_vm, idx, &t);
		sq_addref(_vm, &t);
		sq_release(_vm, &_o);
		_o = t;
	}

	///
	/// Release (any) reference and reset _o.
	///
	void reset(void)
	{
		sq_release(_vm, &_o);
		sq_resetobject(&_o);
	}

	ScriptObject clone()
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

	SQObjectType getType()
	{
		return _o._type;
	}

	//
	// userdata handling
	//

	bool newUserData(const SQChar* key, SQInteger size, SQUserPointer* typetag=0);
	bool getUserData(const SQChar* key, SQUserPointer* data, SQUserPointer* typetag=0);
	bool rawGetUserData(const SQChar* key, SQUserPointer* data, SQUserPointer* typetag=0);

	//
	// arrays handling
	//

	bool arrayResize(SQInteger newSize)
	{
		jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
		sq_pushobject(_vm, getObjectHandle());
		bool res = sq_arrayresize(_vm, -1, newSize) == SQ_OK;
		sq_pop(_vm, 1);
		return res;
	}

	bool arrayExtend(SQInteger amount)
	{
		jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
		SQInteger newLen = length() + amount;
		return arrayResize(newLen);
	}

	bool arrayReverse(void)
	{
		jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
		sq_pushobject(_vm, getObjectHandle());
		bool res = sq_arrayreverse(_vm, -1) == SQ_OK;
		sq_pop(_vm, 1);
		return res;
	}

	ScriptObject arrayPop(SQBool returnPoppedVal=SQTrue)
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

	void arrayAppend(const ScriptObject& o)
	{
		jkSCRIPT_LOGIC_ASSERT(sq_isarray(_o));
		sq_pushobject(_vm, _o);
		sq_pushobject(_vm, o._o);
		sq_arrayappend(_vm, -2);
		sq_pop(_vm, 1);
	}


	int length() const
	{
		jkSCRIPT_LOGIC_ASSERT((sq_isarray(_o) || sq_istable(_o) || sq_isstring(_o)));
		sq_pushobject(_vm, _o);
		int ret = sq_getsize(_vm, -1);
		sq_pop(_vm, 1);
		return ret;
	}

	bool setInstanceUp(SQUserPointer up)
	{
		jkSCRIPT_LOGIC_ASSERT(sq_isinstance(_o));
		sq_pushobject(_vm, _o);
		sq_setinstanceup(_vm, -1, up);
		sq_pop(_vm, 1);
		return true;
	}

	SQUserPointer getInstanceUp(SQUserPointer tag) const
	{
		SQUserPointer up = 0;
		sq_pushobject(_vm, _o);
		sq_getinstanceup(_vm, -1, (SQUserPointer*)&up, tag);
		sq_pop(_vm, 1);
		return up;
	}

	bool isNull() const
	{
		return sq_isnull(_o);
	}

	int isNumeric() const
	{
		return sq_isnumeric(_o);
	}

	bool setDelegate(ScriptObject &obj)
	{
		jkSCRIPT_LOGIC_ASSERT((obj.getType() == OT_TABLE) || (obj.getType() == OT_NULL));
		jkSCRIPT_LOGIC_ASSERT((_o._type == OT_USERDATA) || (_o._type == OT_TABLE));
		sq_pushobject(_vm, _o);
		sq_pushobject(_vm, obj._o);
		jkSCRIPT_API_VERIFY(sq_setdelegate(_vm, -2));
		return true;
	}

	ScriptObject getDelegate()
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

	const SQChar* toString()
	{
		return sq_objtostring(&_o);
	}

	SQBool toBool()
	{
		return sq_objtobool(&_o);
	}

	SQInteger toInteger()
	{
		return sq_objtointeger(&_o);
	}

	SQFloat toFloat()
	{
		return sq_objtofloat(&_o);
	}

	bool getTypeTag(SQUserPointer* typeTag)
	{
		if (SQ_SUCCEEDED(sq_getobjtypetag(&_o, typeTag))) {
			return true;
		}
		return false;
	}

	HSQOBJECT& getObjectHandle()
	{
		return _o;
	}

	HSQOBJECT& handle()
	{
		return _o;
	}

	bool exists(const SQChar* key) const
	{
		bool ret = false;
		if(_getSlot(key)) {
			ret = true;
		}
		sq_pop(_vm, 1);
		return ret;
	}

	ScriptObject getValue(const SQChar* key) const
	{
		ScriptObject ret(_vm);
		if(_getSlot(key)) {
			ret.attachToStackObject(-1);
			sq_pop(_vm, 1);
		}
		sq_pop(_vm, 1);
		return ret;
	}

	///
	/// By-string setters
	///

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

	bool setValue(const SQChar* key, const ScriptObject& val)
	{
		_SETVALUE_STR_BEGIN
		sq_pushobject(_vm, val._o);
		_SETVALUE_STR_END
	}

	bool setValue(const SQChar* key, SQInteger n)
	{
		_SETVALUE_STR_BEGIN
		sq_pushinteger(_vm, n);
		_SETVALUE_STR_END
	}

#undef _SETVALUE_STR_BEGIN
#undef _SETVALUE_STR_END

/*	bool setValue(const ScriptObject &key,const ScriptObject &val);
	bool setValue(SQInteger key,const ScriptObject &val);
	bool setValue(SQInteger key,bool b); // Compiler treats SQBool as INT.
	bool setValue(SQInteger key,SQInteger n);
	bool setValue(SQInteger key,SQFloat f);
	bool setValue(SQInteger key,const SQChar *s);

	bool setValue(const SQChar *key,bool b);
	bool setValue(const SQChar *key,SQInteger n);
	bool setValue(const SQChar *key,SQFloat f);
	bool setValue(const SQChar *key,const SQChar *s);

	bool setUserPointer(const SQChar * key,SQUserPointer up);
	SQUserPointer getUserPointer(const SQChar * key);
	bool setUserPointer(SQInteger key,SQUserPointer up);
	SQUserPointer getUserPointer(SQInteger key);

	ScriptObject GetValue(const SQChar *key) const;
	FLOAT GetFloat(const SQChar *key) const;
	INT GetInt(const SQChar *key) const;
	const SQChar *GetString(const SQChar *key) const;
	bool GetBool(const SQChar *key) const;
	ScriptObject GetValue(INT key) const;
	FLOAT GetFloat(INT key) const;
	INT GetInt(INT key) const;
	const SQChar *GetString(INT key) const;
	bool GetBool(INT key) const;
	ScriptObject GetAttributes(const SQChar *key = NULL);
	BOOL BeginIteration();
	BOOL Next(ScriptObject &key,ScriptObject &value);
	void EndIteration();*/

private:
/*	BOOL GetSlot(const SQChar *name) const;
	BOOL RawGetSlot(const SQChar *name) const;
	BOOL GetSlot(INT key) const;*/

	bool _getSlot(const xchar* name) const
	{
		sq_pushobject(_vm, _o);
		sq_pushstring(_vm, name, -1);
		if(SQ_SUCCEEDED(sq_get(_vm, -2))) {
			return true;
		}

		return false;
	}

	HSQOBJECT _o;
	HSQUIRRELVM _vm;
};

}   //namespace detail
}   //namespace script

#endif//___SCRIPT_SCRIPTOBJECT___
