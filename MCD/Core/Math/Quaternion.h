#ifndef __MCD_CORE_MATH_QUATERNION__
#define __MCD_CORE_MATH_QUATERNION__

#include "Vec4.h"

namespace MCD {

template<typename T>
struct QuaternionTupleUnion {
	union {
		struct { T x, y, z, w; };
		T Data[4];
	};
};	// QuaternionTupleUnion

/*!	Quaternion
	\sa MathTuple
 */
template<typename T>
class Quaternion : public MathTuple<T, 4, Quaternion<T>, QuaternionTupleUnion<T> >
{
    typedef MathTuple<T, 4, Quaternion<T>, QuaternionTupleUnion<T> > super_type;

public:
    typedef typename super_type::param_type param_type;
    using super_type::x;
    using super_type::y;
    using super_type::z;

	inline Quaternion() {}

	explicit Quaternion(const param_type val)
		: super_type(val)
	{}

	Quaternion(const param_type x_, const param_type y_, const param_type z_, const param_type w_) {
		x = x_; y = y_; z = z_; w = w_;
	}

	Quaternion(const Vec4<T>& xyzw) {
		x = xyzw.x; y = xyzw.y; z = xyzw.z; w = xyzw.w;
	}

	Quaternion(const Vec3<T>& xyz, const param_type w_) {
		x = xyz.x; y = xyz.y; z = xyz.z; w = w_;
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

	//! Get the xyz part of the quaternion.
	const Vec3<T>& getVec3() const {
		return reinterpret_cast<const Vec3<T>&>(*this);
	}

	//! Set the xyz part of the quaternion.
	void setVec3(const Vec3<T>& vec3) {
		reinterpret_cast<Vec3<T>&>(*this) = vec3;
	}

	Quaternion conjugate() const {
		return Quaternion(-x, -y, -z, w);
	}

	void mul(const Quaternion& rhs, Quaternion& result) const;

	Quaternion operator*(const Quaternion& rhs) const;

	Quaternion& operator*=(const Quaternion& rhs);

	/*!	
		\param axis Need NOT to be normalized.
	 */
	void fromAxisAngle(const Vec3<T>& axis, float angle);
};	// Quaternion


typedef Quaternion<float> Quaternionf;

}	// namespace MCD

#include "Quaternion.inl"

#endif	// __MCD_CORE_MATH_QUATERNION__
