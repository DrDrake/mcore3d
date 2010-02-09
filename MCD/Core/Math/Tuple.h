#ifndef __MCD_CORE_MATH_TUPLE__
#define __MCD_CORE_MATH_TUPLE__

#include "BasicFunction.h"
#include "../System/TypeTrait.h"
#include <math.h>

namespace MCD {

template<typename T, size_t N>
struct DefaultTupleUnion {
	T data[N];
};	// DefaultTupleUnion

/*!	Tuple structure with common arithmetic operations.
	The main purpose of MathTuple is to unify the code for TVec2, Vec3 etc...
	where those classes will have a common set of arithmetic operations
	like +, -, * and /

	This class must be derived in order to use
	User can pass their own UNION structure (must have a data[N] array as the member variable).

	\note
		Although loop is used in those arithmetic operations, but the compiler should
		able to unroll them.

	Example usage:
	\code
	struct TVec3fTupleUnion {
		union {
			struct { float x, y, z; };
			float data[3];
		};
	};
	struct Vec3f : public MathTuple<float, 3, Vec3f, TVec3fTupleUnion>
	{
		Vec3f(float x_, float y_, float z_) {
			x = x_; y = y_; z = z_;
		}

		// Now we have those +, -, * and / operator defined for Vec3f
	}
	\endcode

	\sa Vec3
 */
template<typename T, size_t N_, class R, class UNION=DefaultTupleUnion<T,N_> >
class MathTuple : public UNION
{
public:
	using UNION::data;

	//! Compile-time meta program may need this enum
	enum { N = N_ };

	//! Parameter type for fast function passing
	typedef typename MCD::ParamType<T>::RET param_type;
	typedef typename MCD::ParamType<const T>::RET const_param_type;

	MathTuple() {}

	MathTuple(const_param_type val) {
		(*this) = val;
	}

	size_t getSize() const {
		return N;
	}

	const_param_type operator[](const size_t i) const
	{
		MCD_ASSUME(i < N);
		return data[i];
	}

	T& operator[](const size_t i)
	{
		MCD_ASSUME(i < N);
		return data[i];
	}

	//! Pointer access for direct copying
	sal_out_ecount(N) T* getPtr() {
		return data;
	}

	//! Pointer access for direct copying
	sal_out_ecount(N) const T* getPtr() const {
		return data;
	}

	//! Assign a value to all of the elements
	R& operator=(const_param_type val)
	{
		for(size_t i=0; i<N; ++i)
			data[i] = val;
		return static_cast<R&>(*this);
	}

	bool operator==(const MathTuple& rhs) const
	{
		for(size_t i=0; i<N; ++i)
			if(data[i] != rhs.data[i])
				return false;
		return true;
	}

	bool operator!=(const MathTuple& rhs) const {
		return !(operator==(rhs));
	}

	//! Positive operator (do nothing)
	const R& operator+() const {
		return static_cast<const R&>(*this);
	}

	//! Negate operator
	R operator-() const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = -data[i];
		return tmp;
	}

	// Arithmetic operations
	R operator+(const MathTuple& rhs) const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = data[i] + rhs.data[i];
		return tmp;
	}

	R operator-(const MathTuple& rhs) const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = data[i] - rhs.data[i];
		return tmp;
	}

	R& operator+=(const MathTuple& rhs)
	{
		for(size_t i=0; i<N; ++i)
			data[i] += rhs.data[i];
		return static_cast<R&>(*this);
	}

	R& operator-=(const MathTuple& rhs)
	{
		for(size_t i=0; i<N; ++i)
			data[i] -= rhs.data[i];
		return static_cast<R&>(*this);
	}

	R& operator*=(const_param_type rhs)
	{
		for(size_t i=0; i<N; ++i)
			data[i] *= rhs;
		return static_cast<R&>(*this);
	}

	R& operator/=(const_param_type rhs) {
		return (*this) *= (T(1.0) / rhs);
	}

	friend bool operator==(const_param_type lhs, const MathTuple& rhs) {
		return MathTuple(lhs) == rhs;
	}

	friend bool operator!=(const_param_type lhs, const MathTuple& rhs) {
		return !(lhs == rhs);
	}

	bool isNearEqual(const MathTuple& rhs, T tolerance = 1e-06) const
	{
		for(size_t i=0; i<N; ++i) {
			if(!Math<T>::isNearEqual(data[i], rhs[i], tolerance))
				return false;
		}
		return true;
	}

	friend R operator+(const MathTuple& lhs, const_param_type rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = lhs.data[i] + rhs;
		return tmp;
	}

	friend R operator+(const_param_type lhs, const MathTuple& rhs) {
		return rhs + lhs;
	}

	friend R operator-(const MathTuple& lhs, const_param_type rhs) {
		return lhs + (-rhs);
	}

	friend R operator-(const_param_type lhs, const MathTuple& rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = lhs - rhs.data[i];
		return tmp;
	}

	friend R operator*(const MathTuple& lhs, const_param_type rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = lhs.data[i] * rhs;
		return tmp;
	}

	friend R operator*(const_param_type lhs, const MathTuple& rhs) {
		return rhs * lhs;
	}

	friend R operator/(const MathTuple& lhs, const_param_type rhs) {
		return lhs * (T(1.0) / rhs);
	}

	friend R operator/(const_param_type lhs, const MathTuple& rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.data[i] = lhs / rhs.data[i];
		return tmp;
	}
};	// MathTuple

}	// namespace MCD

#endif	// __MCD_CORE_MATH_TUPLE__
