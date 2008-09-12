#ifndef __MCD_CORE_MATH_VEC4__
#define __MCD_CORE_MATH_VEC4__

#include "Vec3.h"

namespace MCD {

template<typename T>
struct Vec4TupleUnion {
	union {
		struct { T x, y, z, w; };
		T data[4];
	};
};	// Vec4TupleUnion

/*!	Vector with 4 elements
	\sa MathTuple
 */
template<typename T>
class Vec4 : public MathTuple<T, 4, Vec4<T>, Vec4TupleUnion<T> >
{
    typedef MathTuple<T, 4, Vec4<T>, Vec4TupleUnion<T> > super_type;

public:
    typedef typename super_type::param_type param_type;
    using super_type::x;
    using super_type::y;
    using super_type::z;
    using super_type::w;

	inline Vec4() {}

	explicit Vec4(const param_type val)
		: super_type(val)
	{}

	Vec4(const param_type x_, const param_type y_, const param_type z_, const param_type w_) {
		x = x_; y = y_; z = z_; w = w_;
	}

	Vec4(const Vec3<T>& xyz, const param_type w_) {
		x = xyz.x; y = xyz.y; z = xyz.z; w = w_;
	}

	friend param_type dot(const Vec4& v1, const Vec4& v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}

	//! Dot product operator
	param_type operator%(const Vec4& rhs) const {
		return dot(*this, rhs);
	}

	/*!	Returns the length (magnitude) of the vector.
		\warning
			This operation requires a square root and is expensive in
			terms of CPU operations. If you don't need to know the exact
			length (e.g. for just comparing lengths) use squaredLength()
			instead.
	 */
	Magnitude<param_type> length() const {
		return Magnitude<param_type>(squaredLength());
	}

	/*!	Returns the square of the length(magnitude) of the vector.
		\sa Length
	 */
	param_type squaredLength() const {
		return dot(*this, *this);
	}

	/*!	Returns the distance to another vector.
		\warning
			This operation requires a square root and is expensive in
			terms of CPU operations. If you don't need to know the exact
			distance (e.g. for just comparing distances) use squaredDistance()
			instead.
	 */
	param_type distance(const Vec4& rhs) const {
		return (*this - rhs).length();
	}

	/*!	Returns the square of the distance to another vector.
		\sa Distance
	 */
	param_type squaredDistance(const Vec4& rhs) const {
		return (*this - rhs).squaredLength();
	}

	static const Vec4 cZero;
	static const Vec4 c1000;
	static const Vec4 c0100;
	static const Vec4 c0010;
	static const Vec4 c0001;
};	// Vec4

template<typename T> const Vec4<T> Vec4<T>::cZero = 0;
template<typename T> const Vec4<T> Vec4<T>::c1000 = Vec4(1, 0, 0, 0);
template<typename T> const Vec4<T> Vec4<T>::c0100 = Vec4(0, 1, 0, 0);
template<typename T> const Vec4<T> Vec4<T>::c0010 = Vec4(0, 0, 1, 0);
template<typename T> const Vec4<T> Vec4<T>::c0001 = Vec4(0, 0, 0, 1);

typedef Vec4<float> Vec4f;

}	// namespace MCD

#endif	// __MCD_CORE_MATH_VEC4__
