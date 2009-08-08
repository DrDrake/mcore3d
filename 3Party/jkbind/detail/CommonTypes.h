#ifndef ___SCRIPT_COMMON_TYPES___
#define ___SCRIPT_COMMON_TYPES___

#include "CommonPointers.h"
#include "Classes.h"

namespace script {

// User may wrap around a parameter type to tell the 
// script system to give up the object ownership
template<typename T>
struct GiveUpOwnership
{
	GiveUpOwnership(T v) : value(v) {}
	operator T() { return value; }
	T value;
};

namespace types {

//
// push functions - to pass values into script
//

inline void push(HSQUIRRELVM v,char value)				{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned char value)		{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,short value)				{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned short value)	{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,int value)				{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned int value)		{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,long value)				{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,unsigned long value)		{ sq_pushinteger(v,value); }
inline void push(HSQUIRRELVM v,double value)			{ sq_pushfloat(v,(SQFloat)value); }
inline void push(HSQUIRRELVM v,float value)				{ sq_pushfloat(v,(SQFloat)value); }
inline void push(HSQUIRRELVM v,const SQChar* value)		{ sq_pushstring(v,value,-1); }
inline void push(HSQUIRRELVM v,bool value)				{ sq_pushbool(v,value); }

/*!	The binding engine use this function to associate scripting handle
	to the supplied cpp object, providing a strong linkage between a
	cpp object and it's scripting counterpart.

	\note User may override this function to enable linkage with their own
		cpp object.
	\sa pushHandleFromObject
 */
inline void addHandleToObject(HSQUIRRELVM v, void* obj, int idx)	{ (void)v; (void)obj; (void)idx; jkSCRIPT_LOGIC_ASSERTION(obj != NULL); }

/*!	This function shoud work in pair with addHandleToObject(). While
	addHandleToObject() put handle to the cpp object, this function
	tries to get back that handle from a cpp object and push it onto
	the stack.

	\return True if a handle can be found from the given cpp object, otherwise false.
	\note User may override this function.
	\sa addHandleToObject
 */
inline bool pushHandleFromObject(HSQUIRRELVM v, void* obj)			{ (void)v; (void)obj; jkSCRIPT_LOGIC_ASSERTION(obj != NULL); return false; }

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
	typedef typename ptr::pointer<T>::HostType HostType;
	HostType* p = ptr::pointer<T>::to(obj);

	if(!p) {
		sq_pushnull(v);
		return;
	}

	// Try to use any stored handle in the cpp first
	if(pushHandleFromObject(v, p))
		return;

	// If none has found, push a new one
	ClassID classID = getClassIDFromObject(p, ClassTraits<HostType>::classID());
	detail::ClassesManager::createObjectInstanceOnStackPure(v, classID, p);
	addHandleToObject(v, p, -1);
}

//
// match functions - to check type of the argument from script
//

template<typename T> class TypeSelect {};

// See http://squirrel-lang.org/forums/thread/2674.aspx on SQOBJECT_NUMERIC
inline bool	match(TypeSelect<bool>, HSQUIRRELVM v,int idx)				{ return sq_gettype(v,idx) == OT_BOOL; }
inline bool	match(TypeSelect<char>, HSQUIRRELVM v,int idx)				{ return sq_gettype(v,idx) == OT_INTEGER; }
inline bool	match(TypeSelect<unsigned char>, HSQUIRRELVM v, int idx)	{ return sq_gettype(v,idx) == OT_INTEGER; }
inline bool	match(TypeSelect<short>, HSQUIRRELVM v,int idx)				{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<unsigned short>, HSQUIRRELVM v,int idx)	{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<int>, HSQUIRRELVM v,int idx)				{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<unsigned int>, HSQUIRRELVM v,int idx)		{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<long>, HSQUIRRELVM v,int idx)				{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<unsigned long>, HSQUIRRELVM v,int idx)		{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<float>, HSQUIRRELVM v,int idx)				{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<double>, HSQUIRRELVM v,int idx)			{ return (sq_gettype(v,idx) & SQOBJECT_NUMERIC) > 0; }
inline bool	match(TypeSelect<const SQChar*>, HSQUIRRELVM v,int idx)		{ return sq_gettype(v,idx) == OT_STRING; }

template<typename T>
inline bool match(TypeSelect<const T*>, HSQUIRRELVM v,int idx)			{ return sq_gettype(v,idx) == OT_INSTANCE; }

template<typename T>
inline bool match(TypeSelect<T*>, HSQUIRRELVM v,int idx)				{ return sq_gettype(v,idx) == OT_INSTANCE; }

template<typename T>
inline bool match(TypeSelect<const T&>, HSQUIRRELVM v,int idx)			{ return sq_gettype(v,idx) == OT_INSTANCE; }

template<typename T>
inline bool match(TypeSelect<T&>, HSQUIRRELVM v,int idx)				{ return sq_gettype(v,idx) == OT_INSTANCE; }

template<typename T>
inline bool match(TypeSelect<GiveUpOwnership<T> >,HSQUIRRELVM v,int idx){ return match(TypeSelect<T>(), v, idx); }

//
// get functions - to get values from the script
//

inline bool				get(TypeSelect<bool>,HSQUIRRELVM v,int idx)				{ SQBool b; jkSCRIPT_API_VERIFY(sq_getbool(v,idx,&b)); return b != 0; }
inline char				get(TypeSelect<char>,HSQUIRRELVM v,int idx)				{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<char>(i); }
inline unsigned char	get(TypeSelect<unsigned char>,HSQUIRRELVM v,int idx)	{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned char>(i); }
inline short			get(TypeSelect<short>,HSQUIRRELVM v,int idx)			{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<short>(i); }
inline unsigned short	get(TypeSelect<unsigned short>,HSQUIRRELVM v,int idx)	{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned short>(i); }
inline int				get(TypeSelect<int>,HSQUIRRELVM v,int idx)				{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return i; }
inline unsigned int		get(TypeSelect<unsigned int>,HSQUIRRELVM v,int idx)		{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned int>(i); }
inline long				get(TypeSelect<long>,HSQUIRRELVM v,int idx)				{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<long>(i); }
inline unsigned long	get(TypeSelect<unsigned long>,HSQUIRRELVM v,int idx)	{ SQInteger i; jkSCRIPT_API_VERIFY(sq_getinteger(v,idx,&i)); return static_cast<unsigned long>(i); }
inline float			get(TypeSelect<float>,HSQUIRRELVM v,int idx)			{ SQFloat f; jkSCRIPT_API_VERIFY(sq_getfloat(v,idx,&f)); return f; }
inline double			get(TypeSelect<double>,HSQUIRRELVM v,int idx)			{ SQFloat f; jkSCRIPT_API_VERIFY(sq_getfloat(v,idx,&f)); return static_cast<double>(f); }
inline const SQChar*	get(TypeSelect<const SQChar*>,HSQUIRRELVM v,int idx)	{ const SQChar* s; jkSCRIPT_API_VERIFY(sq_getstring(v,idx,&s)); return s; }

template<typename T>
inline const T&			get(TypeSelect<const T&>, HSQUIRRELVM v,int idx)		{ T* p; jkSCRIPT_API_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); return *p; }

template<typename T>
inline T&				get(TypeSelect<T&>, HSQUIRRELVM v,int idx)				{ T* p; jkSCRIPT_API_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); return *p; }

template<typename T>
inline const T*			get(TypeSelect<const T*>, HSQUIRRELVM v,int idx)		{ T* p; jkSCRIPT_API_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); return p; }

template<typename T>
inline T*				get(TypeSelect<T*>, HSQUIRRELVM v,int idx)				{ T* p; jkSCRIPT_API_VERIFY(sq_getinstanceup(v, idx, (SQUserPointer*)&p, ClassTraits<T>::classID())); return p; }

template<typename T>
inline T				get(TypeSelect<GiveUpOwnership<T> >,HSQUIRRELVM v,int i){ return get(TypeSelect<T>(), v, i); }

//
// type transformation for using with getter/setter functions
//

//! For a generic object, the getter should return the object as a pointer.
template<typename T> struct GetterSetter {
	typedef T type;
	typedef type* getterType;
	typedef type& setterType;
	static inline getterType get(type& val) { return &val; }
};
template<> struct GetterSetter<bool> {
	typedef bool type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<char> {
	typedef char type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned char> {
	typedef unsigned char type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<short> {
	typedef short type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned short> {
	typedef unsigned short type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<int> {
	typedef int type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned int> {
	typedef unsigned int type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<long> {
	typedef long type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<unsigned long> {
	typedef unsigned long type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<float> {
	typedef float type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<double> {
	typedef double type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};
template<> struct GetterSetter<const SQChar*> {
	typedef const SQChar* type;
	typedef type getterType;
	typedef type setterType;
	static inline getterType get(type val) { return val; }
};

}	//namespace types
}	//namespace script

#endif//___SCRIPT_COMMON_TYPES___
