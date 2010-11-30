#include "Pch.h"
#include "../../MCD/Core/System/Platform.h"

#include "../../MCD/Core/System/Atomic.h"
#include "../../MCD/Core/System/IntrusivePtr.h"
#include <utility>

namespace ABC {

/*!	The default proxy object to be used by SharedPtr and WeakPtr.
	Handles reference counter and deletion of object.
 */
template<typename T>
class SharedPtrProxyObject
{
public:
	explicit SharedPtrProxyObject(sal_in_opt T* p=nullptr) : mPtr(p), mRefCount(0), mUseCount(0) {}

	~SharedPtrProxyObject() {
		MCD_ASSERT(mRefCount == 0);
		MCD_ASSERT(mUseCount == 0);
	}

	T* getPtrIncrementUseCount() {
		if(mUseCount > 0) {
			incrementUseCount();
			return mPtr;
		}
		return nullptr;
	}

	void incrementUseCount() {
		++mUseCount;
	}

	void decrementUseCount() {
		if(--mUseCount == 0)
			delete mPtr;
	}

	friend void intrusivePtrAddRef(sal_notnull SharedPtrProxyObject* p) {
		++(p->mRefCount);
	}

	friend void intrusivePtrRelease(sal_notnull SharedPtrProxyObject* p) {
		if(--(p->mRefCount) == 0)
			delete p;
	}

	T* mPtr;

	//! The reference counters. Use atomic integer for thread-safty
	MCD::AtomicInteger mRefCount;	//! Manage the life of SharedPtrProxyObject itself
	MCD::AtomicInteger mUseCount;	//! Manage the life of pointee object
};	// SharedPtrProxyObject

template<class T, class Proxy> class WeakPtr;

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
class SharedPtr : protected MCD::IntrusivePtr<Proxy>
{
	typedef SharedPtr<T> this_type;
	typedef IntrusivePtr<Proxy> Super;
	friend class WeakPtr<T, Proxy>;

public:
	SharedPtr() : Super(new Proxy), mPtr(nullptr) {}

	MCD_IMPLICIT SharedPtr(sal_in_opt T* p)
		: Super(new Proxy(p)), mPtr(p)
	{
		if(p)
			Super::getNotNull()->incrementUseCount();
	}

	//!
	explicit SharedPtr(const Super& proxy)
		: Super(proxy), mPtr(proxy->getPtrIncrementUseCount())
	{
	}

	template<typename U>
	SharedPtr(const SharedPtr<U>& rhs)
		: Super(rhs), mPtr(rhs.get())
	{
		Super::getNotNull()->incrementUseCount();
	}

	SharedPtr(const SharedPtr& rhs)
		: Super(rhs), mPtr(rhs.get())
	{
		Super::getNotNull()->incrementUseCount();
	}

	~SharedPtr()
	{
		if(mPtr)
			Super::getNotNull()->decrementUseCount();
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

	sal_maybenull T* get() const {
		return mPtr;
	}

	//! Use this when you sure the SharedPtr is not null to suppress compiler warning
	sal_notnull T* getNotNull() const {
		return mPtr;
	}

	T& operator*() const {
		return *mPtr;
	}

	//sal_maybenull
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
		return Super::getNotNull()->mUseCount;
	}

private:
	/*!	This is a cached copy of the pointer embedded in the proxy object for fast access.
		If this variable is removed due to memory constrain, one more indirection will
		introduced in the accessor functions.
	 */
	T* mPtr;
};	// SharedPtr

template<class T, class Proxy = SharedPtrProxyObject<T> >
class WeakPtr
{
	typedef WeakPtr<T, Proxy> this_type;
	typedef SharedPtr<T, Proxy> _SharedPtr;
	typedef MCD::IntrusivePtr<Proxy> ProxyPtr;

public:
	MCD_IMPLICIT WeakPtr(const _SharedPtr& ptr = _SharedPtr()) : mProxy(ptr) {}

	template<class U>
	explicit WeakPtr(const WeakPtr<U, Proxy>& rhs) : mProxy(rhs.mProxy) {}

	WeakPtr(const WeakPtr& rhs) : mProxy(rhs.mProxy) {}

	_SharedPtr lock() const {
		return _SharedPtr(mProxy);
	}

	void swap(WeakPtr& rhs)
	{
		ProxyPtr tmp = mProxy;
		mProxy = rhs.mProxy;
		rhs.mProxy = tmp;
	}

private:
	ProxyPtr mProxy;
};	// WeakPtr

/*
template<class T, class U> inline
bool operator==(const WeakPtr<T>& a, const WeakPtr<U>& b) {
	return a.get() == b.get();
}

template<class T, class U> inline
bool operator!=(const WeakPtr<T>& a, const WeakPtr<U>& b) {
	return a.get() != b.get();
}

template<class T, class U> inline
bool operator<(const WeakPtr<T>& a, const WeakPtr<U>& b) {
	return a.get() < b.get();
}

template<class T, class U> inline
bool operator==(const WeakPtr<T>& a, sal_in_opt U* b) {
	return a.get() == b;
}

template<class T, class U> inline
bool operator!=(const WeakPtr<T>& a, sal_in_opt U* b) {
	return a.get() != b;
}

template<class T, class U> inline
bool operator==(sal_in_opt T* a, const WeakPtr<U>& b) {
	return a == b.get();
}

template<class T, class U> inline
bool operator!=(sal_in_opt T* a, const WeakPtr<U>& b) {
	return a != b.get();
}

template<class T> inline
void swap(WeakPtr<T>& lhs, WeakPtr<T>& rhs) {
	lhs.swap(rhs);
}*/

}	// namespace ABC

#include <memory>
#include <fstream>

int main(int, char const*[])
{
#ifdef MCD_VC
	// Tell the c-run time to do memory check at program shut down
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	{
		std::tr1::weak_ptr<int> wp0; 
		std::cout << "wp0.expired() == " << std::boolalpha 
			<< wp0.expired() << std::endl; 

		std::tr1::shared_ptr<int> sp1(new int(5)); 
		std::tr1::weak_ptr<int> wp1(sp1); 
		std::cout << "*wp1.lock() == " 
			<< *wp1.lock() << std::endl; 

		std::tr1::weak_ptr<int> wp2(wp1); 
		std::cout << "*wp2.lock() == " 
			<< *wp2.lock() << std::endl;
	}

	{
		ABC::SharedPtr<int> p(new int(5));
		int a = *p;
		a = a;

		ABC::WeakPtr<int> w(p);
		a = 0;
		ABC::SharedPtr<int> p2 = w.lock();
		p = nullptr;
		a = *p2;
		p2 = nullptr;
		a = 0;
	}

//	return 0;

	size_t ret = 0;
	CppTestHarness::TestRunner runner;
//	runner.ShowTestName(true);

	ret = runner.RunTest("Difference_AnimationClipTest");
	ret = runner.RunTest("AnimationBlendTreeTestFixture::Xml");
	ret = runner.RunAllTests();

	return int(ret);
}
