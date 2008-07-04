#ifndef __MCD_CORE_MATH_TUPLE__
#define __MCD_CORE_MATH_TUPLE__

#include "../System/TypeTrait.h"
#include <math.h>

namespace MCD {

template<typename T, size_t N>
struct DefaultTupleUnion {
	T Data[N];
};	// DefaultTupleUnion

/*!	Tuple structure with common arithmetic operations.
	The main purpose of MathTuple is to unify the code for TVec2, Vec3 etc...
	where those classes will have a common set of arithmetic operations
	like +, -, * and /

	This class must be derived in order to use
	User can pass their own UNION structure (must have a Data[N] array as the member variable).

	\note
		Although loop is used in those arithmetic operations, but the compiler should
		able to unroll them.

	Example usage:
	\code
	struct TVec3fTupleUnion {
		union {
			struct { float x, y, z; };
			float Data[3];
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
    using UNION::Data;

public:
	//! Compile-time meta program may need this enum
	enum { N = N_ };

	//! Parameter type for fast function passing
	typedef typename MCD::ParamType<T>::RET param_type;

	MathTuple() {}

	MathTuple(const param_type val) {
		(*this) = val;
	}

	size_t getSize() const {
		return N;
	}

	param_type operator[](const size_t i) const
	{
		MCD_ASSUME(i < N);
		return Data[i];
	}

	T& operator[](const size_t i)
	{
		MCD_ASSUME(i < N);
		return Data[i];
	}

	//! Pointer access for direct copying
	T* getPtr() {
		return Data;
	}

	//! Pointer access for direct copying
	const T* getPtr() const {
		return Data;
	}

	//! Assign a value to all of the elements
	R& operator=(const param_type val)
	{
		for(size_t i=0; i<N; ++i)
			Data[i] = val;
		return reinterpret_cast<R&>(*this);
	}

	bool operator==(const MathTuple& rhs) const
	{
		for(size_t i=0; i<N; ++i)
			if(Data[i] != rhs.Data[i])
				return false;
		return true;
	}

	bool operator!=(const MathTuple& rhs) const {
		return !(operator==(rhs));
	}

	//! Positive operator (do nothing)
	const R& operator+() const {
		return reinterpret_cast<R&>(*this);
	}

	//! Negate operator
	R operator-() const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = -Data[i];
		return tmp;
	}

	// Arithmetic operations
	R operator+(const MathTuple& rhs) const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = Data[i] + rhs.Data[i];
		return tmp;
	}

	R operator-(const MathTuple& rhs) const
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = Data[i] - rhs.Data[i];
		return tmp;
	}

	R& operator+=(const MathTuple& rhs)
	{
		for(size_t i=0; i<N; ++i)
			Data[i] += rhs.Data[i];
		return reinterpret_cast<R&>(*this);
	}

	R& operator-=(const MathTuple& rhs)
	{
		for(size_t i=0; i<N; ++i)
			Data[i] -= rhs.Data[i];
		return reinterpret_cast<R&>(*this);
	}

	R& operator*=(const param_type rhs)
	{
		for(size_t i=0; i<N; ++i)
			Data[i] *= rhs;
		return reinterpret_cast<R&>(*this);
	}

	R& operator/=(const param_type rhs) {
		return (*this) *= (T(1.0) / rhs);
	}

	friend R operator+(const MathTuple& lhs, const param_type rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = lhs.Data[i] + rhs;
		return tmp;
	}

	friend R operator+(const param_type lhs, const MathTuple& rhs) {
		return rhs + lhs;
	}

	friend R operator-(const MathTuple& lhs, const param_type rhs) {
		return lhs + (-rhs);
	}

	friend R operator-(const param_type lhs, const MathTuple& rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = lhs - rhs.Data[i];
		return tmp;
	}

	friend R operator*(const MathTuple& lhs, const param_type rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = lhs.Data[i] * rhs;
		return tmp;
	}

	friend R operator*(const param_type lhs, const MathTuple& rhs) {
		return rhs * lhs;
	}

	friend R operator/(const MathTuple& lhs, const param_type rhs) {
		return lhs * (T(1.0) / rhs);
	}

	friend R operator/(const param_type lhs, const MathTuple& rhs)
	{
		R tmp;
		for(size_t i=0; i<N; ++i)
			tmp.Data[i] = lhs / rhs.Data[i];
		return tmp;
	}
};	// MathTuple

}	// namespace MCD

#endif	// __MCD_CORE_MATH_TUPLE__
