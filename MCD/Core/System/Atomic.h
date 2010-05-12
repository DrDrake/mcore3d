#ifndef __MCD_CORE_SYSTEM_ATOMIC__
#define __MCD_CORE_SYSTEM_ATOMIC__

#include "Mutex.h"
#include "PlatformIncludeFwd.h"
#include "TypeTrait.h"

namespace MCD {

/*!	A thread safe variable of any type. Protects assignments with a mutex.
 */
template<typename T, typename TArg=typename ParamType<const T>::RET>
class AtomicValue
{
public:
	AtomicValue() : mVal() {}

	explicit AtomicValue(TArg val) : mVal(val) {}

	AtomicValue(const AtomicValue& rhs) {
		set(rhs.get());
	}

	AtomicValue& operator=(const AtomicValue& rhs)
	{
		set(rhs.get());
		return *this;
	}

	AtomicValue& operator=(TArg val)
	{
		set(val);
		return *this;
	}

	operator T() const {
		return get();
	}

	T get() const
	{
		// Rather than simply "return mVal;", we use a more explicit protection
		T ret;
		{	ScopeLock lock(mMutex);
			ret = mVal;
		}
		return ret;
	}

	void set(TArg val)
	{
		ScopeLock lock(mMutex);
		mVal = val;
	}

protected:
	volatile T mVal;
	mutable Mutex mMutex;
};	// AtomicValue

//!	An atomic integer class for performing increment and decrement operations.
class AtomicInteger
{
public:
	AtomicInteger(int i=0) { value=i; }

	//! Atomically pre-increment
	inline int operator++();

	//! Atomically post-increment
	int operator++(int) {
		return ++(*this) - 1;
	}

	//! Atomically pre-decrement
	inline int operator--();

	//! Atomically post-decrement
	int operator--(int) {
		return --(*this) + 1;
	}

	operator int() const {
		return value;
	}
private:
	volatile int value;
};	// AtomicInteger

#if defined(MCD_VC)

int AtomicInteger::operator++() {
	return _InterlockedIncrement((LONG*)&value);
}

int AtomicInteger::operator--() {
	return _InterlockedDecrement((LONG*)&value);
}

#elif defined(MCD_GCC)

// Reference: http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html#Atomic-Builtins
// Reference: http://golubenco.org/2007/06/14/atomic-operations
int AtomicInteger::operator++() {
	return __sync_add_and_fetch((long*)&value, 1);
}

int AtomicInteger::operator--() {
	return __sync_sub_and_fetch((long*)&value, 1);
}

#endif

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_ATOMIC__
