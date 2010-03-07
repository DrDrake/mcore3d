#ifndef ___SCRIPT_CUSTOM_TYPES___
#define ___SCRIPT_CUSTOM_TYPES___

#include <string>

namespace script {
namespace types {

inline bool match(TypeSelect<stdSTRING>, HSQUIRRELVM v, int idx) {
	return sq_gettype(v,idx) == OT_STRING;
}
inline stdSTRING get(TypeSelect<stdSTRING>, HSQUIRRELVM v, int idx) {
	const SQChar* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	return s;
}
template<> struct GetterSetter<stdSTRING> {
	typedef stdSTRING type;
	typedef type getterType;
	typedef type setterType;
	static inline type& get(type& val) { return val; }
};

inline void push(HSQUIRRELVM v, const stdSTRING& value) {
	sq_pushstring(v, value.c_str(), SQInteger(value.length()));
}
inline bool match(TypeSelect<const stdSTRING&>, HSQUIRRELVM v, int idx) {
	return sq_gettype(v,idx) == OT_STRING;
}
inline stdSTRING get(TypeSelect<const stdSTRING&>, HSQUIRRELVM v, int idx) {
	const SQChar* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	return s;
}
template<> struct GetterSetter<const stdSTRING> {
	typedef const stdSTRING type;
	typedef type getterType;
	static inline type& get(type& val) { return val; }
};

template<typename T>
static void destroy(T* obj) {
	delete obj;
}

}	//namespace types
}	//namespace script

#endif//___SCRIPT_CUSTOM_TYPES___