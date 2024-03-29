#ifndef __MCD_CORE_MATH_MAT44__
#define __MCD_CORE_MATH_MAT44__

#include "Vec3.h"
#include "Vec4.h"
#include "Mat33.h"

namespace MCD {

template<typename T>
struct Mat44TupleUnion {
	union {
		// Individual elements
		struct { T
			m00, m10, m20, m30,
			m01, m11, m21, m31,
			m02, m12, m22, m32,
			m03, m13, m23, m33;
		};
		// Columns of Vec4
		struct { T
			c0[4], c1[4], c2[4], c3[4];
		};
		// As a 1 dimension array
		T data[4*4];
		// As a 2 dimension array
		T data2D[4][4];
	};
};	// Mat44TupleUnion

/*!	A 4 by 4 matrix, column-major, column-order (vector-on-the right).
	Vector multiplication: column-order, matrix * column vector, tradition math representation.
	Meory storage order: column-order, in order to keep the memory data DirectX and OpenGL compitable if we use column major.

	If the matrix is used as a transformation matrix, it is assumed to be
	a SRT transform, that is: Scaling, Rotation and Translation.

	Reference: Demystifying Matrix Layouts
	http://www.allbusiness.com/science-technology/mathematics/12809327-1.html
 */
template<typename T>
class Mat44 : public MathTuple<T, 4*4, Mat44<T>, Mat44TupleUnion<T> >
{
	typedef MathTuple<T, 4*4, Mat44<T>, Mat44TupleUnion<T> > super_type;

public:
	// NOTE: I want to get rid of these noisy type forwarding, but GCC keeps complain without it.
	typedef typename super_type::param_type param_type;
	typedef typename super_type::const_param_type const_param_type;
	using super_type::c0;	using super_type::c1;	using super_type::c2;	using super_type::c3;
	using super_type::m00;	using super_type::m01;	using super_type::m02;	using super_type::m03;
	using super_type::m10;	using super_type::m11;	using super_type::m12;	using super_type::m13;
	using super_type::m20;	using super_type::m21;	using super_type::m22;	using super_type::m23;
	using super_type::m30;	using super_type::m31;	using super_type::m32;	using super_type::m33;
	using super_type::data;
	using super_type::data2D;

	enum { N = super_type::N };

	/*!	For performance reasons, default consturctor will not do anything,
		so user must aware of it and do proper initialization afterwards if needed.
	 */
	inline Mat44() {}

	explicit Mat44(const_param_type val)
		: super_type(val)
	{}

	Mat44(const Vec4<T>& col0, const Vec4<T>& col1, const Vec4<T>& col2, const Vec4<T>& col3) {
		r0 = col0; r1 = col1; r2 = col2; r3 = col3;
	}

	Mat44(
		const_param_type m00_, const_param_type m01_, const_param_type m02_, const_param_type m03_,
		const_param_type m10_, const_param_type m11_, const_param_type m12_, const_param_type m13_,
		const_param_type m20_, const_param_type m21_, const_param_type m22_, const_param_type m23_,
		const_param_type m30_, const_param_type m31_, const_param_type m32_, const_param_type m33_)
	{
		m00 = m00_; m01 = m01_; m02 = m02_; m03 = m03_;
		m10 = m10_; m11 = m11_; m12 = m12_; m13 = m13_;
		m20 = m20_; m21 = m21_; m22 = m22_; m23 = m23_;
		m30 = m30_; m31 = m31_; m32 = m32_; m33 = m33_;
	}

	//!	Creates a standard 4x4 transformation matrix with a zero translation part from a rotation / scaling 3x3 matrix.
	explicit Mat44(const Mat33<T>& mat33);

	void copyFrom(sal_in_ecount(16) const T* dataPtr);

	void copyTo(sal_out_ecount(16) T* dataPtr) const;

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

	Mat44& operator*=(const_param_type rhs);

	void mul(const Vec4<T>& rhs, Vec4<T>& result) const;

	Vec4<T> operator*(const Vec4<T>& rhs) const;

//	friend Mat44<T> operator/(const_param_type lhs, const MathTuple& rhs) {
//	}

	void transpose(Mat44& result) const;
	Mat44 transpose() const;

	T determinant() const;

	//! The output parameter \em result can be alias with this.
	sal_checkreturn bool inverse(Mat44& result) const;
	Mat44 inverse() const;

	Vec3<T> xBiasVector() const;
	Vec3<T> yBiasVector() const;
	Vec3<T> zBiasVector() const;

	//!	Get the translation transformation part of the matrix.
	Vec3<T> translation() const;

	//!	Sets the translation transformation part of the matrix.
	void setTranslation(const Vec3<T>& translation);

	//! Apply a delta translation to the current translation.
	void translateBy(const Vec3<T>& deltaTranslation);

	/*!	Get only the scale part from the matri, without the influence of the rotation part.
		Assuming the matrix is composed as (S * R * T).
	 */
	Vec3<T> scale() const;

	/*!	Sets only the scale part of the matrix, without screwing up the rotation part.
		Assuming the matrix is composed as (S * R * T).
		\note Use scaleBy() instead of setScale() whenever possible, because it's more efficient.
	 */
	void setScale(const Vec3<T>& scale);

	//! Apply a delta scale to the current scale, without affacting the rotation part.
	void scaleBy(const Vec3<T>& deltaScale);

	//! Sets only the rotation part of the matrix, without scrwing up the scaling part.
	void setRotation(const Vec3<T>& axis, T angle);

	//! Apply a delta rotation to the current orientation, without affacting the sacling part.
	void rotateBy(const Vec3<T>& axis, T angle);

	//! Rotate this matrix to look at the targeting point.
	void lookAt(const Vec3<T>& lookAt, const Vec3<T>& upVector);

	//! Make an object lookAt matrix, to make a camera lookAt matrix simply inverse the object lookAt matrix.
	void lookAt(const Vec3<T>& eyeAt, const Vec3<T>& lookAt, const Vec3<T>& upVector);

	//!	Extracts the rotation / scaling part of the matrix as a 3x3 matrix.
	void mat33(Mat33<T>& matrix33) const;
	Mat33<T> mat33() const;

	//!	Set the rotation / scaling part of the matrix.
	void setMat33(const Mat33<T>& matrix33);

	void transformPoint(Vec3<T>& point) const;

	void transformPointPerspective(Vec3<T>& point) const;

	void transformNormal(Vec3<T>& normal) const;

	static Mat44 makeScale(const Vec3<T>& scale);

	static Mat44 makeAxisRotation(const Vec3<T>& axis, T angle);

	static Mat44 makeTranslation(const Vec3<T>& translation);

	static const Mat44 cIdentity;
};	// Mat44

typedef Mat44<float> Mat44f;

}	// namespace MCD

#include "Mat44.inl"

#endif	// __MCD_CORE_MATH_MAT33__
