#ifndef __MCD_CORE_MATH_VEC3__
#define __MCD_CORE_MATH_VEC3__

#include "Tuple.h"
#include "Magnitude.h"

namespace MCD {

template<typename T>
struct Vec3TupleUnion {
	union {
		struct { T x, y, z; };
		T data[3];
	};
};	// Vec3TupleUnion

/*!	Vector with 3 elements
	\sa MathTuple
 */
template<typename T>
class Vec3 : public MathTuple<T, 3, Vec3<T>, Vec3TupleUnion<T> >
{
    typedef MathTuple<T, 3, Vec3<T>, Vec3TupleUnion<T> > super_type;

public:
    typedef typename super_type::param_type param_type;
    using super_type::x;
    using super_type::y;
    using super_type::z;

	inline Vec3() {}

	explicit Vec3(const param_type val)
		: super_type(val)
	{}

	Vec3(const param_type x_, const param_type y_, const param_type z_) {
		x = x_; y = y_; z = z_;
	}

	/*!	Calculates the dot (scalar) product of this vector with another.
		The dot product can be used to calculate the angle between 2
		vectors. If both are unit vectors, the dot product is the
		cosine of the angle; otherwise the dot product must be
		divided by the product of the lengths of both vectors to get
		the cosine of the angle.
	 */
	T dot(const Vec3& rhs) const;

	//! Dot product operator
	T operator%(const Vec3& rhs) const;

	//! Dot product of itself.
	T norm() const;

	/*! Calculates the cross-product of 2 vectors, i.e. the vector that
		lies perpendicular to them both.
		\note The resulting vector will <b>NOT</b> be normalized, to maximize efficiency.
		\note
			A right handed coordinate system is used therefore
			Unit Y cross unit Z = unit X, whilst unit Z cross unit Y = - unit X.
	 */
	void cross(const Vec3& rhs, Vec3& result) const;

	Vec3 cross(const Vec3& rhs) const;

	//! Cross product operator.
	Vec3 operator^(const Vec3& rhs) const;

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
	T distance(const Vec3& rhs) const;

	/*!	Returns the square of the distance to another vector.
		\sa Distance
	 */
	T squaredDistance(const Vec3& rhs) const;

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
	Vec3 normalizedCopy() const;

	static const Vec3 cZero;
	static const Vec3 c100;
	static const Vec3 c010;
	static const Vec3 c001;
};	// Vec3

typedef Vec3<float> Vec3f;

}	// namespace MCD

#include "Vec3.inl"

#endif	// __MCD_CORE_MATH_VEC3__
