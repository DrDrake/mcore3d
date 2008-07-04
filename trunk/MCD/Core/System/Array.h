#ifndef __MCD_CORE_SYSTEM_ARRAY__
#define __MCD_CORE_SYSTEM_ARRAY__

#include "Platform.h"

namespace MCD {

/*! Array class with checked limits.
	A replacement for plan old array, adapted from boost's array.hpp
	The index operator[] has range check in debug mode.
 */
template<class T, size_t N_>
class Array
{
public:
	enum { N = N_ };
	T elems[N];	//! Fixed-size array of elements of type T

	T& operator[](size_t i)
	{
		MCD_ASSUME(i < N);
#ifdef NDEBUG
		rangeCheck(i);
#endif
		return elems[i];
	}

	const T& operator[](size_t i) const
	{
		MCD_ASSUME(i < N);
#ifdef NDEBUG
		rangeCheck(i);
#endif
		return elems[i];
	}

	//! Element access without range check, faster but use with care.
	T& unsafeGetAt(size_t i) {
		MCD_ASSUME(i < N);
		return elems[i];
	}

	const T& unsafeGetAt(size_t i) const {
		MCD_ASSUME(i < N);
		return elems[i];
	}

	T& front() {
		return elems[0];
	}

	const T& front() const {
		return elems[0];
	}

	T& back() {
		return elems[N-1];
	}

	const T& back() const {
		return elems[N-1];
	}

	static size_t size() {
		return N;
	}

	//! Direct access to data
	T* data() {
		return elems;
	}

	const T* data() const {
		return elems;
	}

	//! Assign one value to all elements
	void assign(const T& value)
	{
		for(size_t i=0; i<size(); ++i)
			elems[i] = value;
	}

private:
	static void rangeCheck(size_t i)
	{
		if(i >= Array::size())
			throw std::out_of_range();
	}
};	// Array

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_ARRAY__
