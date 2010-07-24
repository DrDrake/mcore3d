#ifndef __MCD_CORE_SYSTEM_WEAKPTR__
#define __MCD_CORE_SYSTEM_WEAKPTR__

#include "IntrusivePtr.h"
#include "Mutex.h"
#include <utility>

namespace MCD {

//!	\sa IntrusiveWeakPtr
class IntrusiveWeakPtrFlag
{
public:
	IntrusiveWeakPtrFlag(bool isValid=true)
		: mRefCount(0), mIsValid(isValid)
	{}

	void setValid(bool flag) {
		mIsValid = flag;
	}

	bool isValid() const {
		return mIsValid;
	}

	friend void intrusivePtrAddRef(sal_notnull IntrusiveWeakPtrFlag* p) {
		++(p->mRefCount);
	}

	friend void intrusivePtrRelease(sal_notnull IntrusiveWeakPtrFlag* p) {
		if(--(p->mRefCount) == 0)
			delete p;
	}

	Mutex& destructionMutex() const {
		return mMutex;
	}

private:
	size_t mRefCount;		//!< Reference counter for the flag
	bool mIsValid;			//!< Flag indicating the object is alive or not
	mutable Mutex mMutex;	//!< Help to ensure the
};	// IntrusiveWeakPtrFlag

template<class T> class IntrusiveWeakPtr;

//!	\sa IntrusiveWeakPtr
class IntrusiveWeakPtrTarget
{
	template<class T> friend class IntrusiveWeakPtr;

public:
	IntrusiveWeakPtrTarget()
		: mValidityFlag(new IntrusiveWeakPtrFlag(true))
	{
	}

	// Prevent copy of other object's flag
	IntrusiveWeakPtrTarget(const IntrusiveWeakPtrTarget&)
		: mValidityFlag(new IntrusiveWeakPtrFlag(true))
	{
	}

	IntrusiveWeakPtrTarget& operator=(const IntrusiveWeakPtrTarget&) {
		// Do nothing
		return *this;
	}

	void destructionLock() {
		mValidityFlag.getNotNull()->destructionMutex().lock();
	}

protected:
	// Protected to prevent deletion via base class (non-virtual destructor)
	~IntrusiveWeakPtrTarget()
	{
		IntrusiveWeakPtrFlag& flag = *mValidityFlag.getNotNull();
		MCD_ASSERT(flag.destructionMutex().isLocked() && "Make sure destructionLock() is called before detroying IntrusiveWeakPtrTarget");
		flag.setValid(false);
		flag.destructionMutex().unlock();
	}

private:
	const IntrusivePtr<IntrusiveWeakPtrFlag>& validityFlag() const {
		return mValidityFlag;
	}

	IntrusivePtr<IntrusiveWeakPtrFlag> mValidityFlag;
};	// IntrusiveWeakPtrTarget

/*!	Convience class for combining shared object and weak pointer target,
	will handle destructionLock() automatically.
 */
template<typename CounterType>
class MCD_ABSTRACT_CLASS IntrusiveSharedWeakPtrTarget : public IntrusiveWeakPtrTarget
{
public:
	IntrusiveSharedWeakPtrTarget() : mRefCount(0) {}

	virtual ~IntrusiveSharedWeakPtrTarget() {}

	friend void intrusivePtrAddRef(IntrusiveSharedWeakPtrTarget* p) {
		++(p->mRefCount);
	}

	friend void intrusivePtrRelease(IntrusiveSharedWeakPtrTarget* p) {
		// NOTE: Gcc4.2 failed to compile "--(p->mRefCount)" correctly.
		p->mRefCount--;
		if(p->mRefCount == 0) {
			p->destructionLock();	// NOTE: We preform the lock before deleting.
			delete p;
		}
	}

protected:
	mutable CounterType mRefCount;
};	// IntrusiveSharedWeakPtrTarget

/*!	A weak pointer class to avoid dangling pointer.
	Introduction:
	A plain pointer can validly be in one of two states - null or pointing to a valid object.
	When a pointer's target is deleted, it and any other pointers that referred to the original
	storage space become invalid. Obviously, you shouldn't dereference a pointer once it has
	become invalid, and the standard actually says that any use of an invalid pointer value is
	undefined behavior.

	In contrast to plain pointers, the IntrusiveWeakPtr can validly be in one of three distinct
	states: null, pointing to a valid object or target object destroyed. In order to achieve
	this, the target object's destructor informs all relevant weak pointers that their target
	has disappeared. The weak pointers then catch any access after their target object has
	been destroyed, and can signal the erroneous usage in a defined manner.

	Implementation:
	The weak pointer is simple to implement using an intrusive model, in which objects that can
	be pointed to by a weak pointer provide a separate "validity" flag. The target object's
	constructor allocates the flag and initializes it to true (i.e. "valid") and the destructor
	resets the flag to false. This means that the flag remains true throughout the lifetime of
	the target object, and false thereafter. In order to ensure that the validity flag itself
	doesn't disappear, any weak pointers that require the flag maintain a counted reference to it.

	The diagram below shows what this looks like for two weak pointers referencing a single target
	object. On the left, the target object and each of the weak pointer objects have a counted
	reference to the validity flag.

	WeakPtr------|        WeakPtr--->?
		|        |            |
		Y        Y            Y
	   true<---target         false
		^        ^            ^
		|        |            |
	WeakPtr------|        WeakPtr--->?

	Destruction of the target object results in the situation shown on the right of the above diagram.
	The target object's destructor has cleared the validity flag, but the two remaining references
	prevent the flag from disappearing. The plain pointers now have indeterminate values
	(indicated by the question marks) and the weak pointers prevent any access to them.

	Note that the weak pointers make no assumptions about how the target object was allocated,
	so it is even possible to use them for objects with automatic or static storage duration.
	This can be useful if you are retro-fitting the pointers to existing code, or for cases where
	you want to detect errors but allow various object management methods.

	Usage:
	In order for a class to be eligible as a weak pointer target, it must provide access to
	an object validity flag via the	validityFlag method returning IntrusivePtr<IntrusiveWeakPtrFlag>.
	This flag must be initialized true and remain true until the object is destroyed, at which
	point it must be set to false. It is also very important that every object have its own flag,
	distinct from every other object's, including copies made via a copy constructor or
	assignment operator.

	To make adding this support as easy as possible, the IntrusiveWeakPtrTarget is provided that client
	classes can use by public inheritance. Note that this is just one possible implementation of the
	requirements, provided for convenience. The IntrusiveWeakPtr template does not require that the target
	class derive from IntrusiveWeakPtrTarget, only that it has a compatible method called validityFlag.

	Thread safety:
	The constructors and assignment operators have to initialize 2 variables (the pointer and the flag),
	but as long as the flag is set correctly, there is no problem on de-referencing the weak pointer.

	But trouble kick in when there is a "strong" smart pointer in one thread, and a weak pointer in another.
	Then if the second thread executes something like:

	weakPtr->someFunction();

	while the pointer in the first thread gets destroyed, the whole sequence can happen like this:

	1. In thread 2, the weak pointer gets dereferenced, and T* is returned.
	2. In thread 1, the strong pointer gets destroyed, and destructs the object.
	3. In thread 2, the weak pointer calls someFunction using the now invalid pointer.

	In the above scenario, user have to synchronize the flow or make sure the object will not
	be destroyed before 'someFunction()' is invoked.

	\sa http://home.clara.net/raoulgough/weak_ptr/index.html
	\sa http://www.jelovic.com/articles/cpp_without_memory_errors_slides.htm
	\sa http://www.jelovic.com/articles/smart_pointer_thread_safety.htm
 */
template<class T>
class IntrusiveWeakPtr
{
	typedef IntrusiveWeakPtr<T> this_type;

public:
	//! Raw pointer constructor (handles null pointer).
	MCD_IMPLICIT IntrusiveWeakPtr(sal_in_opt T* ptr = nullptr) :
		mPtr(ptr),
		mValidityFlag(ptr == nullptr ? new IntrusiveWeakPtrFlag(false) : ptr->validityFlag())
	{
	}

	//! Object reference constructor (guaranteed non-null, faster).
	explicit IntrusiveWeakPtr(T& obj) :
		mPtr(&obj),
		mValidityFlag(obj.validityFlag())
	{
	}

	template<class U>
	IntrusiveWeakPtr(const IntrusiveWeakPtr<U>& rhs) :
		mPtr(rhs.get()),	// The plain pointer cannot always be copied, at least deference occur during virtual inheritance casting
		mValidityFlag(rhs.mValidityFlag)
	{
	}

	IntrusiveWeakPtr(const IntrusiveWeakPtr& rhs) :
		mPtr(rhs.mPtr),
		mValidityFlag(rhs.mValidityFlag)
	{
	}

	sal_maybenull T* get() const {
		return mValidityFlag.getNotNull()->isValid() ? mPtr : nullptr;
	}

	//! Use this when you sure the IntrusiveWeakPtr is not null to suppress compiler warning
	sal_notnull T* getNotNull() const {
		T* p = get();
		MCD_ASSUME(p);
		return p;
	}

	T& operator*() const {
		return *getNotNull();
	}

	//sal_maybenull
	T* operator->() const {
		return get();
	}

	typedef T* this_type::*unspecified_bool_type;

	//!	Non-Null test for using "if (p) ..." to check whether p is nullptr.
	operator unspecified_bool_type() const {
		return get() == nullptr ? nullptr : &this_type::mPtr;
	}

	//!	Null test for using "if(!p) ..." to check whether p is nullptr.
	bool operator!() const {
		return get() == nullptr;
	}

	void swap(IntrusiveWeakPtr& rhs)
	{
		// TODO: Would this be thread safe?
		T*temp = get();
		mPtr = rhs.get();
		rhs.mPtr = temp;
		mValidityFlag.swap(rhs.mValidityFlag);
	}

	Mutex& destructionMutex() const {
		return mValidityFlag->destructionMutex();
	}

private:
	T* mPtr;
	IntrusivePtr<IntrusiveWeakPtrFlag> mValidityFlag;
};	// IntrusiveWeakPtr

template<class T, class U> inline
bool operator==(const IntrusiveWeakPtr<T>& a, const IntrusiveWeakPtr<U>& b) {
	return a.get() == b.get();
}

template<class T, class U> inline
bool operator!=(const IntrusiveWeakPtr<T>& a, const IntrusiveWeakPtr<U>& b) {
	return a.get() != b.get();
}

template<class T, class U> inline
bool operator<(const IntrusiveWeakPtr<T>& a, const IntrusiveWeakPtr<U>& b) {
	return a.get() < b.get();
}

template<class T, class U> inline
bool operator==(const IntrusiveWeakPtr<T>& a, sal_in_opt U* b) {
	return a.get() == b;
}

template<class T, class U> inline
bool operator!=(const IntrusiveWeakPtr<T>& a, sal_in_opt U* b) {
	return a.get() != b;
}

template<class T, class U> inline
bool operator==(sal_in_opt T* a, const IntrusiveWeakPtr<U>& b) {
	return a == b.get();
}

template<class T, class U> inline
bool operator!=(sal_in_opt T* a, const IntrusiveWeakPtr<U>& b) {
	return a != b.get();
}

template<class T> inline
void swap(IntrusiveWeakPtr<T>& lhs, IntrusiveWeakPtr<T>& rhs) {
	lhs.swap(rhs);
}

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_WEAKPTR__
