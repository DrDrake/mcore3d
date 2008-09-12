#ifndef __MCD_CORE_MATH_MAT44__
#define __MCD_CORE_MATH_MAT44__

#include "Vec4.h"

namespace MCD {

template<typename T>
struct Mat44TupleUnion {
	union {
		// Individual elements
		struct { T
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33;
		};
		// Rows of Vec4
		struct { T
			r0[4], r1[4], r2[4], r3[4];
		};
		// As a 1 dimension array
		T data[4*4];
		// As a 2 dimension array
		T data2D[4][4];
	};
};	// Mat44TupleUnion

/*!	A 4 by 4 matrix.
 */
template<typename T>
class Mat44 : public MathTuple<T, 4*4, Mat44<T>, Mat44TupleUnion<T> >
{
    typedef MathTuple<T, 4*4, Mat44<T>, Mat44TupleUnion<T> > super_type;

public:
    typedef typename super_type::param_type param_type;
    using super_type::r0;	using super_type::r1;	using super_type::r2;	using super_type::r3;
    using super_type::m00;	using super_type::m01;	using super_type::m02;	using super_type::m03;
    using super_type::m10;	using super_type::m11;	using super_type::m12;	using super_type::m13;
    using super_type::m20;	using super_type::m21;	using super_type::m22;	using super_type::m23;
	using super_type::m30;	using super_type::m31;	using super_type::m32;	using super_type::m33;

	inline Mat44() {}

	explicit Mat44(const param_type val)
		: super_type(val)
	{}

	Mat44(const Vec4<T>& row0, const Vec4<T>& row1, const Vec4<T>& row2, const Vec4<T>& row3) {
		r0 = row0; r1 = row1; r2 = row2; r3 = row3;
	}

	Mat44(
		const param_type m00_, const param_type m01_, const param_type m02_, const param_type m03_,
		const param_type m10_, const param_type m11_, const param_type m12_, const param_type m13_,
		const param_type m20_, const param_type m21_, const param_type m22_, const param_type m23_,
		const param_type m30_, const param_type m31_, const param_type m32_, const param_type m33_)
	{
		m00 = m00_; m01 = m01_; m02 = m02_; m03 = m03_;
		m10 = m10_; m11 = m11_; m12 = m12_; m13 = m13_;
		m20 = m20_; m21 = m21_; m22 = m22_; m23 = m23_;
		m30 = m30_; m31 = m31_; m32 = m32_; m33 = m33_;
	}

	//! Returns the number of rows
	size_t rows() const {
		return 4u;
	}

	//! Returns the number of columns
	size_t columns() const {
		return 4u;
	}

	const Vec4<T>& operator[](const size_t i) const;

	Vec4<T>& operator[](const size_t i);

	void mul(const Mat44& rhs, Mat44& result) const;

	Mat44 operator*(const Mat44& rhs) const;

	Mat44& operator*=(const Mat44& rhs);

	Mat44& operator*=(const param_type rhs);

	void mul(const Vec4<T>& rhs, Vec4<T>& result) const;

	Vec4<T> operator*(const Vec4<T>& rhs) const;

//	friend Mat44<T> operator/(const param_type lhs, const MathTuple& rhs) {
//	}

	void transpose(Mat44& result) const;

	T determinant() const;

	sal_checkreturn bool inverse(Mat44& result) const;
	Mat44 inverse() const;

	/*!	Creates a rotation matrix.
		\param thetaX Rotation of angle in radius, counter clockwise from the x-axis.
		\param thetaY Rotation of angle in radius, counter clockwise from the y-axis.
		\param thetaZ Rotation of angle in radius, counter clockwise from the z-axis.
	 */
	static void rotate(const param_type thetaX, const param_type thetaY, const param_type thetaZ, Mat44& result);

	static Mat44 rotate(const param_type thetaX, const param_type thetaY, const param_type thetaZ);

	static const Mat44 cIdentity;
};	// Mat44

typedef Mat44<float> Mat44f;

}	// namespace MCD

#include "Mat44.inl"

#endif	// __MCD_CORE_MATH_MAT33__
