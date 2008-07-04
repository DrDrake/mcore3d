#ifndef __SGE_CORE_MATH_VEC3__
#define __SGE_CORE_MATH_VEC3__

#include "Tuple.h"
#include "Magnitude.h"

namespace SGE {

template<typename T>
struct Vec3TupleUnion {
	union {
		struct { T x, y, z; };
		T Data[3];
	};
};	// Vec3TupleUnion

/*!	Vector with 3 elements
	\sa TTuple
 */
template<typename T>
class Vec3 : public MathTuple<T, 3, Vec3<T>, Vec3TupleUnion<T> >
{
    typedef MathTuple<T, 3, Vec3<T>, Vec3TupleUnion<T> > super_type;
    using super_type::x;
    using super_type::y;
    using super_type::z;

public:
    typedef typename super_type::param_type param_type;

	inline Vec3() {}

	Vec3(const param_type val)
		: super_type(val)
	{}

	Vec3(const param_type x_, const param_type y_, const param_type z_) {
		x = x_; y = y_; z = z_;
	}

	friend param_type dot(const Vec3& v1, const Vec3& v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	//! Dot product operator
	param_type operator%(const Vec3& rhs) const {
		return dot(*this, rhs);
	}

	friend param_type norm(const Vec3& v) {
		return dot(v, v);
	}

	friend Vec3 cross(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		);
	}

	//! Cross product operator
	Vec3 operator^(const Vec3& rhs) const {
		return cross(*this, rhs);
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
	param_type distance(const Vec3& rhs) const {
		return (*this - rhs).length();
	}

	/*!	Returns the square of the distance to another vector.
		\sa Distance
	 */
	param_type squaredDistance(const Vec3& rhs) const {
		return (*this - rhs).squaredLength();
	}

static const Vec3 cZero;
	static const Vec3 c100;
	static const Vec3 c010;
	static const Vec3 c001;
};	// Vec3

template<typename T> const Vec3<T> Vec3<T>::cZero = 0;
template<typename T> const Vec3<T> Vec3<T>::c100 = Vec3(1, 0, 0);
template<typename T> const Vec3<T> Vec3<T>::c010 = Vec3(0, 1, 0);
template<typename T> const Vec3<T> Vec3<T>::c001 = Vec3(0, 0, 1);

typedef Vec3<float> Vec3f;

}	// namespace SGE

#endif	// __SGE_CORE_MATH_VEC3__
