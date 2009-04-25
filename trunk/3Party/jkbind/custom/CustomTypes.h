#ifndef ___SCRIPT_CUSTOM_TYPES___
#define ___SCRIPT_CUSTOM_TYPES___

#include <string>

namespace script {
namespace types {

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

}	//namespace types
}	//namespace script

#endif//___SCRIPT_CUSTOM_TYPES___