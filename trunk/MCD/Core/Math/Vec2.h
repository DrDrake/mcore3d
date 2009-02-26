#ifndef __MCD_CORE_MATH_VEC2__
#define __MCD_CORE_MATH_VEC2__

#include "Tuple.h"
#include "Magnitude.h"

namespace MCD {

template<typename T>
struct Vec2TupleUnion {
	union {
		struct { T x, y; };
		T data[2];
	};
};	// Vec2TupleUnion

/*!	Vector with 2 elements
	\sa MathTuple	
 */
template<typename T>
class Vec2 : public MathTuple<T, 2, Vec2<T>, Vec2TupleUnion<T> >
{
	typedef MathTuple<T, 2, Vec2<T>, Vec2TupleUnion<T> > super_type;

public:
	typedef typename super_type::param_type param_type;
	using super_type::x;
	using super_type::y;

	inline Vec2() {}

	explicit Vec2(const param_type val)
		: super_type(val)
	{}

	Vec2(const param_type x_, const param_type y_) {
		x = x_; y = y_;
	}

	/*!	Calculates the dot (scalar) product of this vector with another.
		The dot product can be used to calculate the angle between 2
		vectors. If both are unit vectors, the dot product is the
		cosine of the angle; otherwise the dot product must be
		divided by the product of the lengths of both vectors to get
		the cosine of the angle.
	 */
	T dot(const Vec2& rhs) const;

	//! Dot product operator
	T operator%(const Vec2& rhs) const;

	//! Dot product of itself.
	T norm() const;

	/*!	Returns the length (magnitude) of the vector.
		\warning
			This operation requires a square root and is expensive in
			terms of CPU operations. If you don't need to know the exact
			length (e.g. for just comparing lengths) use squaredLength()
			instead.
	 */
	Magnitude<T> length() const;

	/*!	Returns the square of the length(magnitude) of the vector.
		\sa Length
	 */
	T squaredLength() const;

	/*!	Returns the distance to another vector.
		\warning
			This operation requires a square root and is expensive in
			terms of CPU operations. If you don't need to know the exact
			distance (e.g. for just comparing distances) use squaredDistance()
			instead.
	 */
	T distance(const Vec2& rhs) const;

	/*!	Returns the square of the distance to another vector.
		\sa Distance
	 */
	T squaredDistance(const Vec2& rhs) const;

	/*!	Normalizes the vector.
		This method normalizes the vector such that it's length / magnitude is 1.
		The result is called a unit vector.
		\return The previous length of the vector.
		\note This function will crash for zero-sized vectors.
	 */
	T normalize();

	/*!	As normalize(), except that this function will not crash for zero-sized vectors,
		but there will be no changes made to their components.
	 */
	T normalizeSafe();

	/*!	As normalize(), except that this vector is unaffected and the
		normalized vector is returned as a copy.
		\note This function will crash for zero-sized vectors.
	 */
	Vec2 normalizedCopy() const;

	//!	Returns whether this vector is within a positional tolerance of another vector.
	bool isNearEqual(const Vec2& rhs, T tolerance = 1e-06) const;

	static const Vec2 cZero;
	static const Vec2 c10;
	static const Vec2 c01;
};	// Vec2

typedef Vec2<float> Vec2f;

}	// namespace MCD

#include "Vec2.inl"

#endif	// __MCD_CORE_MATH_VEC2__
