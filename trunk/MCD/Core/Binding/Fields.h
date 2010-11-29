#ifndef __MCD_CORE_BINDING_FIELDS__
#define __MCD_CORE_BINDING_FIELDS__

#include "Classes.h"
#include "../System/StringHash.h"

namespace MCD {
namespace Binding {

template<typename T> struct GetterSetter {
	typedef T type;
	typedef type* getterType;	// For a generic object, the getter should return the object as a pointer.
	typedef type& setterType;
	static getterType get(type& val) { return &val; }
};
template<typename T> struct GetterSetter<T*> {
	typedef T type;
	typedef type* getterType;
	typedef type*& setterType;
	static getterType get(type* val) { return val; }
};
template<> struct GetterSetter<bool> {
	typedef bool type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<char> {
	typedef char type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned char> {
	typedef unsigned char type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<short> {
	typedef short type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned short> {
	typedef unsigned short type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<int> {
	typedef int type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned int> {
	typedef unsigned int type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<long> {
	typedef long type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned long> {
	typedef unsigned long type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<float> {
	typedef float type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<double> {
	typedef double type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<const char*> {
	typedef const char* type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(type val) { return val; }
};
template<> struct GetterSetter<std::string> {
	typedef const char* type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(const std::string& val) { return val.c_str(); }
};
template<> struct GetterSetter<const std::string> {
	typedef const char* type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(const std::string& val) { return val.c_str(); }
};
template<> struct GetterSetter<FixString> {
	typedef const char* type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(const FixString& val) { return val.c_str(); }
};
template<> struct GetterSetter<const FixString> {
	typedef const char* type;
	typedef type getterType;
	typedef type setterType;
	static getterType get(const FixString& val) { return val.c_str(); }
};

template<class ResultPolicy, class Callee, class RawField>
SQInteger pushField(Callee* callee, RawField (Callee::*fieldPtr), HSQUIRRELVM v)
{
	MCD_ASSUME(callee);
	typedef GetterSetter<RawField> GetterSetter;
	return ResultPolicy::template pushResult(v, GetterSetter::get(callee->*fieldPtr));
}

template<class Callee, class Field, class ReturnPolicy>
SQInteger fieldGetterFunction(HSQUIRRELVM v)
{
	Callee* instance(nullptr);
	if(SQ_FAILED(fromInstanceUp(v, 1, instance, instance, ClassTraits<Callee>::classID())))
		return sq_throwerror(v, "Trying to get member variable without a correct this pointer");

	Field field = getFieldPointer<Field>(v, -1);
	return pushField<ReturnPolicy, Callee>(instance, field, v);
}

template<class Callee, class RawField>
SQInteger assignField(Callee* callee, RawField (Callee::*fieldPtr), HSQUIRRELVM v, int index)
{
	MCD_ASSUME(callee);
	typedef typename GetterSetter<RawField>::setterType fieldType;
	MCD_VERIFY(match(TypeSelect<fieldType>::adjusted(), v, index));
	callee->*fieldPtr = get(TypeSelect<fieldType>::adjusted(), v, index);
	return 0;
}

template<class Callee, class Field>
SQInteger fieldSetterFunction(HSQUIRRELVM v)
{
	Callee* instance(nullptr);
	if(SQ_FAILED(fromInstanceUp(v, 1, instance, instance, ClassTraits<Callee>::classID())))
		return sq_throwerror(v, "Trying to set member variable without a correct this pointer");

	Field field = getFieldPointer<Field>(v, -1);
	return assignField<Callee>(instance, field, v, 2);
}

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_FIELDS__
