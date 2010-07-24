#ifndef __MCD_CORE_MATH_MAT33__
#define __MCD_CORE_MATH_MAT33__

#include "Vec3.h"

namespace MCD {

template<typename T>
struct Mat33TupleUnion {
	union {
		// Individual elements
		struct { T
			m00, m01, m02,
			m10, m11, m12,
			m20, m21, m22;
		};
		// Rows of Vec3
		struct { T
			r0[3], r1[3], r2[3];
		};
		// As a 1 dimension array
		T data[3*3];
		// As a 2 dimension array
		T data2D[3][3];
	};
};	// Mat33TupleUnion

/*!	A 3 by 3 matrix.
	\note
		The (x,y,z) coordinate system is assumed to be right-handed (Open Gl style)
		  Y
		  |
		  |____ X
		 /
		/
		Z
		Coordinate axis rotation matrices are of the form
		  RX =    1       0       0
				  0     cos(t) -sin(t)
				  0     sin(t)  cos(t)
		where t > 0 indicates a counterclockwise rotation in the yz-plane
		  RY =  cos(t)    0     sin(t)
				  0       1       0
			   -sin(t)    0     cos(t)
		where t > 0 indicates a counterclockwise rotation in the zx-plane
		  RZ =  cos(t) -sin(t)    0
				sin(t)  cos(t)    0
				  0       0       1
		where t > 0 indicates a counterclockwise rotation in the xy-plane.
 */
template<typename T>
class Mat33 : public MathTuple<T, 3*3, Mat33<T>, Mat33TupleUnion<T> >
{
	typedef MathTuple<T, 3*3, Mat33<T>, Mat33TupleUnion<T> > super_type;

public:
	typedef typename super_type::param_type param_type;
	typedef typename super_type::const_param_type const_param_type;
	using super_type::r0;	using super_type::r1;	using super_type::r2;
	using super_type::m00;	using super_type::m01;	using super_type::m02;
	using super_type::m10;	using super_type::m11;	using super_type::m12;
	using super_type::m20;	using super_type::m21;	using super_type::m22;
	using super_type::data;
	using super_type::data2D;

	inline Mat33() {}

	explicit Mat33(const_param_type val)
		: super_type(val)
	{}

	Mat33(const Vec3<T>& row0, const Vec3<T>& row1, const Vec3<T>& row2) {
		r0 = row0; r1 = row1; r2 = row2;
	}

	Mat33(
		const_param_type m00_, const_param_type m01_, const_param_type m02_,
		const_param_type m10_, const_param_type m11_, const_param_type m12_,
		const_param_type m20_, const_param_type m21_, const_param_type m22_)
	{
		m00 = m00_; m01 = m01_; m02 = m02_;
		m10 = m10_; m11 = m11_; m12 = m12_;
		m20 = m20_; m21 = m21_; m22 = m22_;
	}

	//! Returns the number of rows
	size_t rows() const {
		return 3u;
	}

	//! Returns the number of columns
	size_t columns() const {
		return 3u;
	}

	const Vec3<T>& operator[](const size_t i) const;

	Vec3<T>& operator[](const size_t i);

	void mul(const Mat33& rhs, Mat33& result) const;

	Mat33 operator*(const Mat33& rhs) const;

	Mat33& operator*=(const Mat33& rhs);

	Mat33& operator*=(const_param_type rhs);

	void mul(const Vec3<T>& rhs, Vec3<T>& result) const;

	Vec3<T> operator*(const Vec3<T>& rhs) const;

//	friend Mat33<T> operator/(const_param_type lhs, const MathTuple& rhs) {
//	}

	void transpose(Mat33& result) const;
	Mat33 transpose() const;

	T determinant() const;

	/*!	Decompose the scale part from the matrix.
		Assuming the matrix is composed as (S * R).
	 */
	Vec3<T> scale() const;

	/*!	Sets the scale part of the matrix.
		Assuming the matrix is composed as (S * R).
		You can get a pure rotation matrix by setting it's scale to one.
	 */
	void setScale(const Vec3<T>& scale);

	/*!	Gets the Euler angles, as it's rotated in the order of X, Y then Z.
		\param thetaX Rotation of angle in radius, counter clockwise from the x-axis.
		\param thetaY Rotation of angle in radius, counter clockwise from the y-axis.
		\param thetaZ Rotation of angle in radius, counter clockwise from the z-axis.
		\return The convertion gives unique result or not.
	 */
	bool getRotationXYZ(T& thetaX, T& thetaY, T& thetaZ);

	/*!	Creates a rotation matrix from Euler angles, in the order of rotation in X, Y then Z.
		\param thetaX Rotation of angle in radius, counter clockwise from the x-axis.
		\param thetaY Rotation of angle in radius, counter clockwise from the y-axis.
		\param thetaZ Rotation of angle in radius, counter clockwise from the z-axis.
	 */
	static void makeXYZRotation(const_param_type thetaX, const_param_type thetaY, const_param_type thetaZ, Mat33& result);

	static Mat33 makeXYZRotation(const_param_type thetaX, const_param_type thetaY, const_param_type thetaZ);

	/*!	Creates a rotation matrix.
		\param axis The rotation axis.
		\param angle Rotation of angle in radian.
	 */
	static void makeAxisRotation(const Vec3<T>& axis, T angle, Mat33& result);

	static Mat33 makeAxisRotation(const Vec3<T>& axis, T angle);

	static const Mat33 cIdentity;
};	// Mat33

typedef Mat33<float> Mat33f;

}	// namespace MCD

#include "Mat33.inl"

#endif	// __MCD_CORE_MATH_MAT33__
