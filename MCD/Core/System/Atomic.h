#ifndef __SGE_CORE_SYSTEM_ATOMIC__
#define __SGE_CORE_SYSTEM_ATOMIC__

#include "Mutex.h"
#include "PlatformIncludeFwd.h"
#include "TypeTrait.h"

namespace SGE {

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

#ifdef SGE_GCC

inline long interlockedExchangeAdd(long volatile* addEnd, long value)
{
	long ret;
	__asm__ __volatile__(
		"lock xaddl %0,(%1)"
		:"=r" (ret)
		:"r" (addEnd), "0" (value)
		:"memory"
	);
	return ret;
}

#endif

//!	AtomicInteger
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

#if defined(SGE_VC)

int AtomicInteger::operator++() {
	return _InterlockedIncrement((LONG*)&value);
}

int AtomicInteger::operator--() {
	return _InterlockedDecrement((LONG*)&value);
}

#elif defined(SGE_GCC)

int AtomicInteger::operator++() {
	return interlockedExchangeAdd((long*)&value, 1);
}

int AtomicInteger::operator--() {
	return interlockedExchangeAdd((long*)&value, -1);
}

#endif

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_ATOMIC__
