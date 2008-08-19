#ifndef __MCD_CORE_SYSTEM_INTRUSIVEPTR__
#define __MCD_CORE_SYSTEM_INTRUSIVEPTR__

#include "Platform.h"

namespace MCD {

/*!	A smart pointer that uses intrusive reference counting.
	Relies on unqualified calls to
	void intrusivePtrAddRef(T* p);
	void intrusivePtrRelease(T* p);

	It is worth to note that IntrusivePtr made no assumption on the memory allocation/de-allocation
	of the pointee type. That means user are free to use their own memory allocation scheme.

	\note The object is responsible for destroying itself (eg. inside the intrusivePtrRelease() function).
	\note IntrusivePtr is thread safe if operation in intrusivePtrAddRef() and intrusivePtrRelease()
		  are thread safe (eg. use atomic interger for the reference counter)
 */
template<class T>
class IntrusivePtr
{
	typedef IntrusivePtr<T> this_type;

public:
	IntrusivePtr() : mPtr(nullptr)	{}

	MCD_IMPLICIT IntrusivePtr(sal_in_opt T* p, bool addRef=true)
		: mPtr(p)
	{
		if(mPtr != nullptr && addRef)
			intrusivePtrAddRef(mPtr);
	}

	template<class U>
	IntrusivePtr(const IntrusivePtr<U>& rhs)
		: mPtr(rhs.get())
	{
		if(mPtr != nullptr)
			intrusivePtrAddRef(mPtr);
	}

	IntrusivePtr(const IntrusivePtr& rhs)
		: mPtr(rhs.get())
	{
		if(mPtr != nullptr)
			intrusivePtrAddRef(mPtr);
	}

	~IntrusivePtr()
	{
		if(mPtr != nullptr)
			intrusivePtrRelease(mPtr);
	}

	template<class U>
	IntrusivePtr& operator=(const IntrusivePtr<U>& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr& operator=(const IntrusivePtr& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr& operator=(sal_in_opt T* rhs)
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

	typedef T* this_type::*unspecified_bool_type;

	//!	Non-Null test for using "if (p) ..." to check whether p is nullptr.
	operator unspecified_bool_type() const {
		return mPtr == nullptr ? nullptr : &this_type::mPtr;
	}

	//!	Null test for using "if(!p) ..." to check whether p is nullptr.
	bool operator!() const {
		return mPtr == nullptr;
	}

	void swap(IntrusivePtr& rhs)
	{
		T* tmp = mPtr;
		mPtr = rhs.mPtr;
		rhs.mPtr = tmp;
	}

private:
	T* mPtr;
};	// IntrusivePtr

template<class T, class U> inline
bool operator==(const IntrusivePtr<T>& a, const IntrusivePtr<U>& b) {
	return a.get() == b.get();
}

template<class T, class U> inline
bool operator!=(const IntrusivePtr<T>& a, const IntrusivePtr<U>& b) {
	return a.get() != b.get();
}

template<class T, class U> inline
bool operator==(const IntrusivePtr<T>& a, sal_in_opt U* b) {
	return a.get() == b;
}

template<class T, class U> inline
bool operator!=(const IntrusivePtr<T>& a, sal_in_opt U* b) {
	return a.get() != b;
}

template<class T, class U> inline
bool operator==(sal_in_opt T* a, const IntrusivePtr<U>& b) {
	return a == b.get();
}

template<class T, class U> inline
bool operator!=(sal_in_opt T* a, const IntrusivePtr<U>& b) {
	return a != b.get();
}

template<class T> inline
void swap(IntrusivePtr<T>& lhs, IntrusivePtr<T>& rhs) {
	lhs.swap(rhs);
}

template<class T, class U>
IntrusivePtr<T> static_pointer_cast(const IntrusivePtr<U>& p) {
	return static_cast<T*>(p.get());
}

template<class T, class U>
IntrusivePtr<T> const_pointer_cast(const IntrusivePtr<U>& p) {
	return const_cast<T*>(p.get());
}

template<class T, class U>
IntrusivePtr<T> dynamic_pointer_cast(const IntrusivePtr<U>& p) {
	return dynamic_cast<T*>(p.get());
}

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_INTRUSIVEPTR__
