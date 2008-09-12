#ifndef __MCD_CORE_MATH_QUATERNION__
#define __MCD_CORE_MATH_QUATERNION__

#include "Vec4.h"

namespace MCD {

template<typename T>
struct QuaternionTupleUnion {
	union {
		struct { T x, y, z, w; };
		T data[4];
	};
};	// QuaternionTupleUnion

/*!	Quaternions can represent general 3D rotations with 4 floating point numbers.
	Quaternions can describe any rotation about any axis in 3D space and,
	unlike Euler angles, quaternions do not present issues like "gimbal lock".
	See: http://www.gamedev.net/reference/articles/article1095.asp
	Quaternions are small and efficient and are often a good replacement for
	rotation matrices. They take up less space, only 4 scalars as opposed to
	9 for a 3x3 rotation matrix. Quaternion multiplication is also more
	efficient than matrix multiplication and a quaternion can be easily and
	quickly converted to a rotation matrix where necessary. These properties
	make quaternions ideal for many algorithms and systems like for instance
	an animation system. Such an animation system often uses interpolation
	between quaternions to generate rotations in between key frames.
	Different animations can also be blended together to achieve smooth
	transitions from one animation to another. As it turns out interpolation
	between two general rotations is not trivial and can be computationally
	expensive. However, quaternions are generally the best representation
	for interpolating orientations and there are several different approaches
	with different properties and different computational costs.
	\author J.M.P. van Waveren

	A quaternion can be represented as <tt>Q = x*i + y*j + z*k + w</tt>
	where w is a real number, and <tt>x, y, z</tt> are complex numbers
	so that <tt>i^2 = j^2 = k^2 = i*j*k = -1</tt>.

	You can visualize unit quaternions as a rotation in 4D space where the (x,y,z)
	components form the arbitrary axis and the w forms the angle of rotation.
	All the unit quaternions form a sphere of unit length in the 4D space.
	Again, this is not very intuitive but what I'm getting at is that you can
	get a 180 degree rotation of a quaternion by simply inverting the scalar
	(w) component.

	\note Only unit quaternions can be used for representing orientations.
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
    using super_type::w;

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

	/*!	Returns the length (magnitude) of the quaternion.
		\warning
			This operation requires a square root and is expensive in
			terms of CPU operations. If you don't need to know the exact
			length (e.g. for just comparing lengths) use squaredLength()
			instead.
	 */
	Magnitude<param_type> length() const {
		return Magnitude<param_type>(squaredLength());
	}

	/*!	Returns the square of the length(magnitude) of the quaternion.
		\sa Length
	 */
	param_type squaredLength() const {
		return x * x + y * y + z * z + w * w;
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

	/*!	Apply to non-zero quaternion.
		Returns zero if the operation fails.
	 */
	Quaternion inverse() const;

	/*!	Apply to unit quaternion.
		Faster than inverse().
	 */
	Quaternion inverseUnit() const;

	void mul(const Quaternion& rhs, Quaternion& result) const;

	Quaternion operator*(const Quaternion& rhs) const;

	Quaternion& operator*=(const Quaternion& rhs);

	/*!	Creat the quaternion from a rotation axis and the angle.
		\param[in] axis The rotation axis, need NOT to be normalized.
		\param[in] angle The angle of rotation in radian.
	 */
	void fromAxisAngle(const Vec3<T>& axis, T angle);

	/*!	Convert the quaternion to a rotation axis and the angle.
		\param[out] axis The rotation axis, need NOT to be normalized.
		\param[out] angle The angle of rotation in radian.
	 */
	void toAxisAngle(Vec3<T>& axis, T& angle) const;

	/*!	Apply the quaternion rotation transform to the 3D vector.
		v = q * Quaternion(v,1) * q.conjugate()
	 */
	void transform(Vec3<T>& v) const;

	//!	Returns whether this quaternion is within a tolerance of another quaternion.
	bool isNearEqual(const Quaternion& rhs, T tolerance = 1e-06) const;

	/*!	The multiplication identity quaternion, (0, 0, 0, 1).
		Any quaternion multiplied with this identity quaternion will not be changed.
	 */
	static const Quaternion cIdentity;
};	// Quaternion

typedef Quaternion<float> Quaternionf;

}	// namespace MCD

#include "Quaternion.inl"

#endif	// __MCD_CORE_MATH_QUATERNION__
