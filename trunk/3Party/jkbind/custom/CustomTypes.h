#ifndef ___SCRIPT_CUSTOM_TYPES___
#define ___SCRIPT_CUSTOM_TYPES___

#include <string>

namespace script {
namespace types {

inline bool match(TypeSelect<std::wstring>, HSQUIRRELVM v, int idx) {
	return sq_gettype(v,idx) == OT_STRING;
}
inline std::wstring get(TypeSelect<std::wstring>, HSQUIRRELVM v, int idx) {
	const SQChar* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	return s;
}
template<> struct GetterSetter<std::wstring> {
	typedef std::wstring type;
	typedef type getterType;
	typedef type setterType;
	static inline type& get(type& val) { return val; }
};

inline void push(HSQUIRRELVM v, const std::wstring& value) {
	sq_pushstring(v, value.c_str(), SQInteger(value.length()));
}
inline bool match(TypeSelect<const std::wstring&>, HSQUIRRELVM v, int idx) {
	return sq_gettype(v,idx) == OT_STRING;
}
inline std::wstring get(TypeSelect<const std::wstring&>, HSQUIRRELVM v, int idx) {
	const SQChar* s;
	jkSCRIPT_API_VERIFY(sq_getstring(v, idx, &s));
	return s;
}
template<> struct GetterSetter<const std::wstring> {
	typedef const std::wstring type;
	typedef type getterType;
	static inline type& get(type& val) { return val; }
};

}	//namespace types
}	//namespace script

#endif//___SCRIPT_CUSTOM_TYPES___