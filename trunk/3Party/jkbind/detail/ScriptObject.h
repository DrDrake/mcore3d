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

	ScriptObject& operator =(const ScriptObject& o);

	ScriptObject& operator =(int n);

	void attachToStackObject(int idx);

	///
	/// Release (any) reference and reset _o.
	///
	void reset(void);

	ScriptObject clone();

	SQObjectType getType() {
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

	bool arrayResize(SQInteger newSize);

	bool arrayExtend(SQInteger amount);

	bool arrayReverse(void);

	ScriptObject arrayPop(SQBool returnPoppedVal=SQTrue);

	void arrayAppend(const ScriptObject& o);

	int length() const;

	bool setInstanceUp(SQUserPointer up);

	SQUserPointer getInstanceUp(SQUserPointer tag) const;

	bool isNull() const {
		return sq_isnull(_o);
	}

	int isNumeric() const {
		return sq_isnumeric(_o);
	}

	bool setDelegate(ScriptObject &obj);

	ScriptObject getDelegate();

	const SQChar* toString() {
		return sq_objtostring(&_o);
	}

	SQBool toBool() {
		return sq_objtobool(&_o);
	}

	SQInteger toInteger() {
		return sq_objtointeger(&_o);
	}

	SQFloat toFloat() {
		return sq_objtofloat(&_o);
	}

	bool getTypeTag(SQUserPointer* typeTag);

	HSQOBJECT& getObjectHandle() { return _o; }

	HSQOBJECT& handle() { return _o; }

	bool exists(const SQChar* key) const;

	ScriptObject getValue(const SQChar* key) const;

	///
	/// By-string setters
	///

	bool setValue(const SQChar* key, const ScriptObject& val);

	bool setValue(const SQChar* key, SQInteger n);

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

	bool _getSlot(const xchar* name) const;

	HSQOBJECT _o;
	HSQUIRRELVM _vm;
};	// ScriptObject

}   //namespace detail
}   //namespace script

#endif//___SCRIPT_SCRIPTOBJECT___
