#ifndef __MCD_CORE_SYSTEM_ARRAY__
#define __MCD_CORE_SYSTEM_ARRAY__

#include "Platform.h"

namespace MCD {

/*! Array class with checked limits.
	A replacement for plan old array, adapted from boost's array.hpp
	The index operator[] has range check in debug mode.
 */
template<typename T, size_t N_>
class Array
{
public:
	enum { N = N_ };
	T elems[N];	//! Fixed-size array of elements of type T

	T& operator[](size_t i)
	{
		MCD_ASSUME(i < N);
		return elems[i];
	}

	const T& operator[](size_t i) const
	{
		MCD_ASSUME(i < N);
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

	size_t size() const {
		return N;
	}

	//! Direct access to data
	sal_out_ecount(N) T* data() {
		return elems;
	}

	sal_out_ecount(N) const T* data() const {
		return elems;
	}

	//! Assign one value to all elements
	void assign(const T& value)
	{
		for(size_t i=0; i<size(); ++i)
			elems[i] = value;
	}
};	// Array

/*!	An class that wrap around a raw pointer and tread it as an array of type T.
	This class also support array stride, making it very usefull when working with
	vertex buffer.
 */
template<typename T>
class StrideArray
{
public:
	StrideArray(const T* _data, size_t elementCount, size_t _stride=0)
		: data((char*)_data), size(elementCount), stride(_stride == 0 ? sizeof(T) : _stride)
	{
		MCD_ASSERT(stride >= sizeof(T));
	}

	//! Construct from non-const version of StrideArray<T>, U must have the const qualifier.
	template<typename U>
	MCD_IMPLICIT StrideArray(const StrideArray<U>& rhs)
		: data((char*)const_cast<T*>(rhs.getPtr())), size(rhs.size), stride(rhs.stride)
	{}

	T& operator[](size_t i) const
	{
		MCD_ASSUME(i < size);
		return *reinterpret_cast<T*>(data + i*stride);
	}

	T* getPtr() const { return reinterpret_cast<T*>(data); }

	size_t sizeInByte() const { return size * stride; }

	bool isEmpty() const { return !data || size == 0 || stride == 0; }

	char* data;
	size_t size;	//!< Element count.
	size_t stride;
};	// StrideArray

//!	Specialization of StrideArray which give more room for the compiler to do optimization.
template<typename T, size_t stride=sizeof(T)>
class FixStrideArray
{
public:
	FixStrideArray(const T* _data, size_t elementCount)
		: data((char*)_data), size(elementCount)
#ifndef NDEBUG
		, cStride(stride)
#endif
	{}

	//! Construct from non-const version of FixStrideArray<T>, U must have the const qualifier.
	template<typename U>
	MCD_IMPLICIT FixStrideArray(const FixStrideArray<U>& rhs)
		: data((char*)const_cast<T*>(rhs.getPtr())), size(rhs.size)
#ifndef NDEBUG
		, cStride(rhs.stride)
#endif
	{}

	T& operator[](size_t i) const
	{
		MCD_ASSUME(i < size);
		return *reinterpret_cast<T*>(data + i*stride);
	}

	T* getPtr() const { return reinterpret_cast<T*>(data); }

	size_t sizeInByte() const { return size * stride; }

	char* data;
	size_t size;	//!< Element count.

#ifndef NDEBUG
	//! For Visual Studio debugger visualization purpose.
	size_t cStride;
#endif
};	// FixStrideArray

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_ARRAY__
