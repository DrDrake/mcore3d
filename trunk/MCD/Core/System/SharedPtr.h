#ifndef __MCD_CORE_SYSTEM_SHAREDPTR__
#define __MCD_CORE_SYSTEM_SHAREDPTR__

#include "IntrusivePtr.h"

namespace MCD {

/*!	The default proxy object to be used by SharedPtr.
	Handles reference counter and deletion of object for IntrusivePtr.
 */
template<typename T>
class SharedPtrProxyObject
{
public:
	SharedPtrProxyObject() : mPtr(nullptr), mRefCount(0) {}

	SharedPtrProxyObject(sal_in_opt T* p) : mPtr(p), mRefCount(0) {}

	~SharedPtrProxyObject() {
		// It will share the same life with the pointee object.
		delete mPtr;
	}

	T* mPtr;

	//! The reference counter. You may change it to atomic integer for thread-safty
	size_t mRefCount;
};	// SharedPtrProxyObject

template<typename T>
void intrusivePtrAddRef(sal_notnull SharedPtrProxyObject<T>* p) {
	++(p->mRefCount);
}

template<typename T>
void intrusivePtrRelease(sal_notnull SharedPtrProxyObject<T>* p) {
	if(--(p->mRefCount) == 0)
		delete p;
}

/*!	A simple reference counted smart pointer (non-intrusive).
	The implementation of SharedPtr use IntrusivePtr to hold a reference to a proxy object, where
	this proxy object provides the reference counter for the actual object that we want to point to.

	In order to have more control on the type of reference counter used (ie. thread safety) or how
	the pointee object is destroyed, you can make your own proxy class instead of using the default
	SharedPtrProxyObject class.

	Example:
	\code
	SharedPtr<int> p1(new int(123));
	SharedPtr<int> p2 = p1;

	int a = *p1;

	// Please never use raw pointer to communicate between 2 SharedPtr!
	// SharedPtr<int> p3 = p1.get();
	\endcode

	\note
		Non-intrusive smart pointers do not mix well with raw pointers: the moment someone goes from
		a smart pointer to a raw pointer back to a smart pointer, disaster may strike at any moment!

	\note
		Casting among the inheritance tree is not supported. Reworking is need if we want to support
		such feature, currently you are recommended to use IntrusivePtr instead.
 */
template<typename T, class Proxy = SharedPtrProxyObject<T> >
class SharedPtr : protected IntrusivePtr<Proxy>
{
	typedef SharedPtr<T> this_type;
	typedef IntrusivePtr<Proxy> Super;

public:
	SharedPtr() : Super(), mPtr(nullptr) {}

	MCD_IMPLICIT SharedPtr(sal_in_opt T* p, bool addRef=true)
		: Super(new Proxy(p), addRef), mPtr(p)
	{
	}

/*	MCD_IMPLICIT SharedPtr(sal_in Proxy* proxy, bool addRef=true)
		: Super(proxy, addRef), mPtr(proxy->mPtr)
	{
	}*/

	template<typename U>
	SharedPtr(const SharedPtr<U>& rhs)
		: Super(rhs), mPtr(rhs.get())
	{
	}

	SharedPtr(const SharedPtr& rhs)
		: Super(rhs), mPtr(rhs.get())
	{
	}

	template<typename U>
	SharedPtr& operator=(const SharedPtr<U>& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	SharedPtr& operator=(const SharedPtr& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	SharedPtr& operator=(sal_in_opt T* rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	T* get() const {
		return mPtr;
	}

	T& operator*() const {
		return *mPtr;
	}

	T* operator->() const {
		return mPtr;
	}

#ifdef MCD_VC
	//!	Non-Null test for using "if (p) ..." to check whether p is nullptr.
	operator unspecified_bool_type() const {
		return mPtr == nullptr ? nullptr : unspecified_bool_type(&this_type::mPtr);
	}
#else	// We have problem using Super::unspecified_bool_type in gcc
	operator bool() const {
		return mPtr != nullptr;
	}
#endif

	//!	Null test for using "if(!p) ..." to check whether p is nullptr.
	bool operator!() const {
		return mPtr == nullptr;
	}

	void swap(SharedPtr& rhs)
	{
		Super::swap(rhs);
		T* tmp = mPtr;
		mPtr = rhs.mPtr;
		rhs.mPtr = tmp;
	}

	size_t referenceCount() const {
		return Super::get()->mRefCount;
	}

private:
	/*!	This is a cached copy of the pointer embedded in the proxy object for fast access.
		If this variable is removed due to memory constrain, one more indirection will
		introduced in the accessing functions.
	 */
	T* mPtr;
};	// SharedPtr

template<class T, class U, class P1, class P2> inline
bool operator==(const SharedPtr<T,P1>& a, const SharedPtr<U,P2>& b) {
	return a.get() == b.get();
}

template<class T, class U, class P1, class P2> inline
bool operator!=(const SharedPtr<T,P1>& a, const SharedPtr<U,P2>& b) {
	return a.get() != b.get();
}

template<class T, class U, class P> inline
bool operator==(const SharedPtr<T,P>& a, sal_in_opt U* b) {
	return a.get() == b;
}

template<class T, class U, class P> inline
bool operator!=(const SharedPtr<T,P>& a, sal_in_opt U* b) {
	return a.get() != b;
}

template<class T, class U, class P> inline
bool operator==(sal_in_opt T* a, const SharedPtr<U,P>& b) {
	return a == b.get();
}

template<class T, class U, class P> inline
bool operator!=(sal_in_opt T* a, const SharedPtr<U,P>& b) {
	return a != b.get();
}

template<class T, class P> inline
void swap(SharedPtr<T,P>& lhs, SharedPtr<T,P>& rhs) {
	lhs.swap(rhs);
}

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_SHAREDPTR__
