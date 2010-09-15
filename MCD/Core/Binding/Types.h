#ifndef __MCD_CORE_BINDING_TYPES__
#define __MCD_CORE_BINDING_TYPES__

//#include "CommonPointers.h"
#include "Classes.h"

namespace MCD {
namespace Binding {

#define CAPI_VERIFY(arg) MCD_VERIFY(SQ_SUCCEEDED((arg)))

// User may wrap around a parameter type to tell the 
// script system to give up the object ownership
template<typename T>
struct GiveUpOwnership
{
	GiveUpOwnership(T v) : value(v) {}
	operator T() { return value; }
	T value;
};

// push functions - to pass values into script

inline void push(HSQUIRRELVM v,char value)					{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned char value)			{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,short value)					{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned short value)		{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,int value)					{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned int value)			{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,long value)					{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned long value)			{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,double value)				{ sq_pushfloat(v,(SQFloat)value); }
inline void push(HSQUIRRELVM v,float value)					{ sq_pushfloat(v,(SQFloat)value); }
inline void push(HSQUIRRELVM v,bool value)					{ sq_pushbool(v,value); }
inline void push(HSQUIRRELVM v,const char* value)			{ sq_pushstring(v,value,-1); }
inline void push(HSQUIRRELVM v,const std::string& value)	{ sq_pushstring(v,value.c_str(), SQInteger(value.length())); }

/*!	The binding engine use this function to associate scripting handle
	to the supplied cpp object, providing a strong linkage between a
	cpp object and it's scripting counterpart.

	\note User may override this function to enable linkage with their own
		cpp object.
	\sa pushHandleFromObject
 */
inline void addHandleToObject(HSQUIRRELVM v, void* obj, int idx) { (void)v; (void)obj; (void)idx; MCD_ASSERT(obj); }

/*!	This function shoud work in pair with addHandleToObject(). While
	addHandleToObject() put handle to the cpp object, this function
	tries to get back that handle from a cpp object and push it onto
	the stack.

	\return True if a handle can be found from the given cpp object, otherwise false.
	\note User may override this function.
	\sa addHandleToObject
 */
inline bool pushHandleFromObject(HSQUIRRELVM v, void* obj) { (void)v; (void)obj; MCD_ASSERT(obj); return false; }

/*!	This function is invoked at the first time an object is being push into squirrel,
	to tell squirrel what is the actual type of the object.
	The default implementation is to simply return the parameter \em originalID,
	but user can override this function to support their own polymorphic type.

	\sa ClassesManager::associateClassID

	Sample implementation using std map:
	\code
	struct TypeInfo {
		const std::type_info& typeInfo;
		TypeInfo(const std::type_info& t) : typeInfo(t) {}
		bool operator<(const TypeInfo& rhs) const
		{	return typeInfo.before(rhs.typeInfo) > 0;	}
	};	// TypeInfo
	typedef std::map<TypeInfo, ClassID> TypeMap;
	static TypeMap typeMap;

	void associateClassID(const std::type_info& typeInfo, script::ClassID classID)
	{	typeMap[typeInfo] = classID;	}

	ClassID getClassIDFromObject(const MyPolymorphicBaseClass* obj, ClassID original) {
		TypeMap::const_iterator i = typeMap.find(typeid(*obj));
		if(i != typeMap.end())
			return i->second;
		return original;
	}
	\endcode
 */
inline ClassID getClassIDFromObject(void* obj, ClassID originalID) { return originalID; }

template<typename T>
void push(HSQUIRRELVM v, T obj)
{
	typedef typename pointer<T>::HostType HostType;
	HostType* p = pointer<T>::to(obj);

	if(!p) {
		sq_pushnull(v);
		return;
	}

	// Try to use any stored handle in the cpp first
//	if(pushHandleFromObject(v, p))
//		return;

	// If none has found, push a new one
	ClassID classID = getClassIDFromObject(p, ClassTraits<HostType>::classID());
	ClassesManager::createObjectInstanceOnStack(v, classID, p);
//	addHandleToObject(v, p, -1);
}

// match functions - to check type of the argument from script

template<typename T> class TypeSelect {};

// See http://squirrel-lang.org/forums/thread/2674.aspx on SQOBJECT_NUMERIC
inline bool match(TypeSelect<bool>,HSQUIRRELVM v,int idx)							{ return sq_gettype(v,idx) == OT_BOOL; }
inline bool match(TypeSelect<char>,HSQUIRRELVM v,int idx)							{ return sq_gettype(v,idx) == OT_INTEGER; }
inline bool match(TypeSelect<unsigned char>,HSQUIRRELVM v, int idx)					{ return sq_gettype(v,idx) == OT_INTEGER; }
inline bool match(TypeSelect<short>,HSQUIRRELVM v,int idx)							{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<unsigned short>,HSQUIRRELVM v,int idx)					{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<int>,HSQUIRRELVM v,int idx)							{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<unsigned int>,HSQUIRRELVM v,int idx)					{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<long>,HSQUIRRELVM v,int idx)							{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<unsigned long>,HSQUIRRELVM v,int idx)					{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<float>,HSQUIRRELVM v,int idx)							{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<double>,HSQUIRRELVM v,int idx)							{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool match(TypeSelect<const char*>,HSQUIRRELVM v,int idx)					{ return sq_gettype(v,idx) == OT_STRING; }
inline bool match(TypeSelect<std::string>,HSQUIRRELVM v,int idx)					{ return sq_gettype(v,idx) == OT_STRING; }
inline bool match(TypeSelect<const std::string>,HSQUIRRELVM v,int idx)				{ return sq_gettype(v,idx) == OT_STRING; }

template<typename T> bool match(TypeSelect<const T*>,HSQUIRRELVM v,int idx)			{ SQUserPointer p; return SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, ClassTraits<T>::classID())) ? true : sq_gettype(v,idx) == OT_NULL; }
template<typename T> bool match(TypeSelect<T*>,HSQUIRRELVM v,int idx)				{ return match(TypeSelect<const T*>(), v, idx); }
template<typename T> bool match(TypeSelect<const T&>,HSQUIRRELVM v,int idx)			{ SQUserPointer p; bool ok = SQ_SUCCEEDED(sq_getinstanceup(v, idx, &p, ClassTraits<T>::classID())); return ok ? p != nullptr : ok; }
template<typename T> bool match(TypeSelect<T&>,HSQUIRRELVM v,int idx)				{ return match(TypeSelect<const T&>(), v, idx); }
template<typename T> bool match(TypeSelect<GiveUpOwnership<T> >,HSQUIRRELVM v,int idx){ return match(TypeSelect<T>(), v, idx); }

// Get functions - to get values from the script

inline bool				get(TypeSelect<bool>,HSQUIRRELVM v,int idx)					{ SQBool b; CAPI_VERIFY(sq_getbool(v,idx,&b)); return b != 0; }
inline char				get(TypeSelect<char>,HSQUIRRELVM v,int idx)					{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<char>(i); }
inline unsigned char	get(TypeSelect<unsigned char>,HSQUIRRELVM v,int idx)		{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned char>(i); }
inline short			get(TypeSelect<short>,HSQUIRRELVM v,int idx)				{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<short>(i); }
inline unsigned short	get(TypeSelect<unsigned short>,HSQUIRRELVM v,int idx)		{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned short>(i); }
inline int				get(TypeSelect<int>,HSQUIRRELVM v,int idx)					{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return i; }
inline unsigned int		get(TypeSelect<unsigned int>,HSQUIRRELVM v,int idx)			{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned int>(i); }
inline long				get(TypeSelect<long>,HSQUIRRELVM v,int idx)					{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<long>(i); }
inline unsigned long	get(TypeSelect<unsigned long>,HSQUIRRELVM v,int idx)		{ SQInteger i; CAPI_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned long>(i); }
inline float			get(TypeSelect<float>,HSQUIRRELVM v,int idx)				{ SQFloat f; CAPI_VERIFY(sq_getfloat(v,idx,&f)); return f; }
inline double			get(TypeSelect<double>,HSQUIRRELVM v,int idx)				{ SQFloat f; CAPI_VERIFY(sq_getfloat(v,idx,&f)); return static_cast<double>(f); }
inline const char*		get(TypeSelect<const char*>,HSQUIRRELVM v,int idx)			{ const char* s; CAPI_VERIFY(sq_getstring(v,idx,&s)); return s; }
inline std::string		get(TypeSelect<std::string>,HSQUIRRELVM v,int idx)			{ const char* s; CAPI_VERIFY(sq_getstring(v,idx,&s)); return s; }

#ifdef NDEBUG	// No type tag check in release mode, as the check should be already done by the match() function first.
template<typename T> const T&	get(TypeSelect<const T&>,HSQUIRRELVM v,int idx)		{ T* p; sq_getinstanceup(v, idx, (SQUserPointer*)&p, 0); return *p; }
template<typename T> const T*	get(TypeSelect<const T*>,HSQUIRRELVM v,int idx)		{ if(sq_gettype(v,idx) == OT_NULL) return NULL; T* p; sq_getinstanceup(v, idx, (SQUserPointer*)&p, 0); return p; }
#else
template<typename T> const T&	get(TypeSelect<const T&>,HSQUIRRELVM v,int idx)		{ T* p; CAPI_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); MCD_ASSUME(p); return *p; }
template<typename T> const T*	get(TypeSelect<const T*>,HSQUIRRELVM v,int idx)		{ if(sq_gettype(v,idx) == OT_NULL) return NULL; T* p; CAPI_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); return p; }
#endif
template<typename T> T&			get(TypeSelect<T&>,HSQUIRRELVM v,int idx)			{ return const_cast<T&>(get(TypeSelect<const T&>(), v, idx)); }
template<typename T> T*			get(TypeSelect<T*>,HSQUIRRELVM v,int idx)			{ return const_cast<T*>(get(TypeSelect<const T*>(), v, idx)); }
template<typename T> T			get(TypeSelect<GiveUpOwnership<T> >,HSQUIRRELVM v,int i){ return get(TypeSelect<T>(), v, i); }

/// Destroying an object, with the ability for the user to specialize this function even for a base class
/// To specialize for a base class:
/// template<typename T> static void destroy(const Base* dummy, T* obj) { obj->XXX(); }
/// http://stackoverflow.com/questions/1332678/priority-when-choosing-overloaded-template-functions-in-c
template<typename T>
static void destroy(const void* dummy, T* obj) {
	(void)dummy;
	delete obj;
}

// Pointer type handling
template<typename T> struct pointer {
	typedef T HostType;
	/// Invoked when a function returns an object but not a pointer/reference,
	/// and a new instance should be new on heap.
	static HostType* to(const T& src) {
		return new T(src);
	}
};

template<typename T> struct pointer<T*> {
	typedef T HostType;
	static HostType* to(T* src) { return src; }
};

template<typename T> struct pointer<T&> {
	typedef T HostType;
	static HostType* to(T& src) { return &src; }
};

template<typename T> struct pointer<const T*> {
	typedef T HostType;
	static HostType* to(const T* src) { return const_cast<T*>(src); }
};

template<typename T> struct pointer<const T&> {
	typedef T HostType;
	static HostType* to(const T& src) { return const_cast<T*>(&src); }
};

#undef CAPI_VERIFY

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_TYPES__
