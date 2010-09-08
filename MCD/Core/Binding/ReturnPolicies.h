#ifndef __MCD_CORE_BINDING_RETURNPOLICIES__
#define __MCD_CORE_BINDING_RETURNPOLICIES__

/// Return policies of the functions

#include "Types.h"
#include "../../../3Party/squirrel/squirrel.h"

namespace MCD {
namespace Binding {

/// Default policy, used to push plain types to stack
class plain
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		push(v, result);
		return 1;
	}
};	// plain

/// Used to return already on-stack values
/// \note argNum should take the function environment arg into account i.e arg count + 1
template<size_t argNum>
class alreadyAsArg
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		HSQOBJECT existingObj;
		sq_resetobject(&existingObj);
		sq_getstackobj(v, argNum, &existingObj);
		sq_pushobject(v, existingObj);
		return 1;
	}
};	// alreadyAsArg

/// Used to put calee back to stack
class alreadyAsThis: public alreadyAsArg<1>
{
};

/// Creates new instance with no relations with host object lifetime
class objNoCare
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		push(v, result);
		return 1;
	}
};	// objNoCare

/// Construct new instance, and deletes host object on destruction
class construct
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		typedef typename pointer<RT>::HostType HostType;
		HostType* obj = pointer<RT>::to(result);

		// Note that at the time when the C function is invoked, the
		// squirrel side object instance was already on the stack.
		if(!obj || SQ_FAILED(sq_setinstanceup(v, 1, obj)))
			return sq_throwerror(v, "Failed to construct object");

		sq_setreleasehook(v, 1, releaseHook<HostType>);
//		types::addHandleToObject(v, obj, 1);

		return 1;
	}

private:
	template<typename T>
	static SQInteger releaseHook(SQUserPointer p, SQInteger size)
	{
		T* data = (T*)p;
		destroy(data, data);
		return 1;
	}
};	// construct

/// Creates new instance, which deletes host object on destruction
class objOwn
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		push(v, result);
		sq_setreleasehook(v, -1, releaseHook<typename ptr::pointer<RT>::HostType>);
		return 1;
	}

private:
	template<typename T>
	static SQInteger releaseHook(SQUserPointer p,SQInteger size)
	{
		T* data = (T*)p;
		destroy(data, data);
		return 1;
	}
};

/// creates new instance, which control host objects lifetime by custom ptr-class
template<typename PtrClass>
class objPtr
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		types::push(v, result);
//		typedef typename ptr::pointer<RT>::HostType HostType;
//		HostType* p = ptr::pointer<RT>::to(result);
//		detail::ClassesManager::createObjectInstanceOnStackPure(v, ClassTraits<HostType>::classID(), p);

		sq_pushinteger(v, detail::ClassesManager::MEMORY_CONTROLLER_PARAM);
		new(sq_newuserdata(v, sizeof(RT))) RT(result);
		sq_setreleasehook(v, -1, releaseHook<RT>);
//		jkSCRIPT_VERIFY(sq_set(v, -3));

		return 1;
	}

private:
	template<typename T>
	static SQInteger releaseHook(SQUserPointer p, SQInteger size)
	{
		T* data = (T*)p;
		data->~T();
		return 1;
	}
};

/// Creates new instance, which controls host object lifetime by addRef-releaseRef custom methods
template<typename RefPolicy>
class objRefCount
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		typedef typename ptr::pointer<RT>::HostType HostType;
		HostType* obj = ptr::pointer<RT>::to(result);
		RefPolicy::addRef(obj);
		types::push(v, result);
		sq_setreleasehook(v, -1, releaseHook<HostType>);
		return 1;
	}

private:
	template<typename T>
	static SQInteger releaseHook(SQUserPointer p, SQInteger size)
	{
		T* data = (T*)p;
		RefPolicy::releaseRef(data);
		return 1;
	}
};

/// Construct new instance, and returns using objRefCount policy
template<typename RefPolicy>
class constructObjRefCount
{
public:
	template<typename RT>
	static SQInteger pushResult(HSQUIRRELVM v, RT result)
	{
		typedef typename ptr::pointer<RT>::HostType HostType;
		HostType* obj = ptr::pointer<RT>::to(result);

		RefPolicy::addRef(obj);
		// TODO: Support negative indexing
		if(SQ_FAILED(sq_setinstanceup(v, 1, obj)))
			return -1;

		sq_setreleasehook(v, 1, releaseHook<HostType>);
		types::addHandleToObject(v, obj, 1);

		return 1;
	}

private:
	template<typename T>
	static SQInteger releaseHook(SQUserPointer p, SQInteger size)
	{
		T* data = (T*)p;
		RefPolicy::releaseRef(data);
		return 1;
	}
};

/// Default return policies
template<typename T> struct DefaultReturnPolicy			{ typedef objOwn policy; };
template<typename T> struct DefaultReturnPolicy<const T>{ typedef typename DefaultReturnPolicy<T>::policy policy; };
template<> struct DefaultReturnPolicy<bool>				{ typedef plain policy; };
template<> struct DefaultReturnPolicy<char>				{ typedef plain policy; };
template<> struct DefaultReturnPolicy<unsigned char>	{ typedef plain policy; };
template<> struct DefaultReturnPolicy<short>			{ typedef plain policy; };
template<> struct DefaultReturnPolicy<unsigned short>	{ typedef plain policy; };
template<> struct DefaultReturnPolicy<int>				{ typedef plain policy; };
template<> struct DefaultReturnPolicy<unsigned int>		{ typedef plain policy; };
template<> struct DefaultReturnPolicy<long>				{ typedef plain policy; };
template<> struct DefaultReturnPolicy<unsigned long>	{ typedef plain policy; };
template<> struct DefaultReturnPolicy<float>			{ typedef plain policy; };
template<> struct DefaultReturnPolicy<double>			{ typedef plain policy; };
template<> struct DefaultReturnPolicy<const char*>		{ typedef plain policy; };
template<> struct DefaultReturnPolicy<std::string>		{ typedef plain policy; };

// Detect the type of a class member variable
template<typename T>
struct DetectFieldType {};

template<class Callee, typename T>
struct DetectFieldType<T (Callee::*)> {
	typedef T type;
};

/// For a getter function, all policy other than plain should become objNoCare,
/// since the object is already own by the C++ object.
template<typename T> struct GetterReturnPolicy { typedef objNoCare policy; };
template<> struct GetterReturnPolicy<plain> { typedef plain policy; };

}	// namespace Binding
}	// namespace MCD

#endif	// __MCD_CORE_BINDING_RETURNPOLICIES__
