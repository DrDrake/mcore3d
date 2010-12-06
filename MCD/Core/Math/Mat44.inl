#if !defined(MCD_GCC) || defined(__SSE__)
#   include <xmmintrin.h>
#endif

#if defined(MCD_APPLE)
#	include "vfpmath/matrix_impl.h"
#endif

#include <memory.h> // For memcpy

namespace MCD {

template<typename T>
Mat44<T>::Mat44(const Mat33<T>& matrix33)
{
	setMat33(matrix33);
	m03 = m13 = m23 = m30 = m31 = m32 = 0;
	m33 = 1;
}

template<typename T>
void Mat44<T>::copyFrom(const T* dataPtr) {
	::memcpy(this->getPtr(), dataPtr, sizeof(T) * N);
}

template<typename T>
void Mat44<T>::copyTo(T* dataPtr) const {
	::memcpy(dataPtr, this->getPtr(), sizeof(T) * N);
}

template<typename T>
const Vec4<T>& Mat44<T>::operator[](const size_t i) const
{
	MCD_ASSUME(i < columns());
	return *(reinterpret_cast<const Vec4<T>*>(&c0) + i);
}

template<typename T>
Vec4<T>& Mat44<T>::operator[](const size_t i)
{
	MCD_ASSUME(i < columns());
	return *(reinterpret_cast<Vec4<T>*>(&c0) + i);
}

template<typename T>
void Mat44<T>::mul(const Mat44& rhs, Mat44& ret) const
{
	MCD_ASSUME(&rhs != &ret);
	MCD_ASSUME(this != &ret);

#if defined(MCD_IPHONE_DEVICE)

	Matrix4Mul(this->getPtr(), rhs.getPtr(), ret.getPtr());

#elif !defined(MCD_GCC) || defined(__SSE__)

	// Reference for SSE matrix multiplication:
	// http://www.cortstratton.org/articles/OptimizingForSSE.php
	__m128 x4 = _mm_loadu_ps(c0);
	__m128 x5 = _mm_loadu_ps(c1);
	__m128 x6 = _mm_loadu_ps(c2);
	__m128 x7 = _mm_loadu_ps(c3);

	__m128 x0, x1, x2, x3;

	for(size_t i=0; i<4; ++i) {
		x1 = x2 = x3 = x0 = _mm_loadu_ps(rhs.data2D[i]);
		x0 = _mm_shuffle_ps(x0, x0, _MM_SHUFFLE(0,0,0,0));
		x1 = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(1,1,1,1));
		x2 = _mm_shuffle_ps(x2, x2, _MM_SHUFFLE(2,2,2,2));
		x3 = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(3,3,3,3));

		x0 = _mm_mul_ps(x0, x4);
		x1 = _mm_mul_ps(x1, x5);
		x2 = _mm_mul_ps(x2, x6);
		x3 = _mm_mul_ps(x3, x7);

		x2 = _mm_add_ps(x2, x0);
		x3 = _mm_add_ps(x3, x1);
		x3 = _mm_add_ps(x3, x2);

		_mm_storeu_ps(ret.data2D[i], x3);
	}

#else

	T a0, a1, a2, a3;

	a0 = m00; a1 = m01; a2 = m02; a3 = m03;
	ret.m00 = a0 * rhs.m00 + a1 * rhs.m10 + a2 * rhs.m20 + a3 * rhs.m30;
	ret.m01 = a0 * rhs.m01 + a1 * rhs.m11 + a2 * rhs.m21 + a3 * rhs.m31;
	ret.m02 = a0 * rhs.m02 + a1 * rhs.m12 + a2 * rhs.m22 + a3 * rhs.m32;
	ret.m03 = a0 * rhs.m03 + a1 * rhs.m13 + a2 * rhs.m23 + a3 * rhs.m33;

	a0 = m10; a1 = m11; a2 = m12; a3 = m13;
	ret.m10 = a0 * rhs.m00 + a1 * rhs.m10 + a2 * rhs.m20 + a3 * rhs.m30;
	ret.m11 = a0 * rhs.m01 + a1 * rhs.m11 + a2 * rhs.m21 + a3 * rhs.m31;
	ret.m12 = a0 * rhs.m02 + a1 * rhs.m12 + a2 * rhs.m22 + a3 * rhs.m32;
	ret.m13 = a0 * rhs.m03 + a1 * rhs.m13 + a2 * rhs.m23 + a3 * rhs.m33;

	a0 = m20; a1 = m21; a2 = m22; a3 = m23;
	ret.m20 = a0 * rhs.m00 + a1 * rhs.m10 + a2 * rhs.m20 + a3 * rhs.m30;
	ret.m21 = a0 * rhs.m01 + a1 * rhs.m11 + a2 * rhs.m21 + a3 * rhs.m31;
	ret.m22 = a0 * rhs.m02 + a1 * rhs.m12 + a2 * rhs.m22 + a3 * rhs.m32;
	ret.m23 = a0 * rhs.m03 + a1 * rhs.m13 + a2 * rhs.m23 + a3 * rhs.m33;

	a0 = m30; a1 = m31; a2 = m32; a3 = m33;
	ret.m30 = a0 * rhs.m00 + a1 * rhs.m10 + a2 * rhs.m20 + a3 * rhs.m30;
	ret.m31 = a0 * rhs.m01 + a1 * rhs.m11 + a2 * rhs.m21 + a3 * rhs.m31;
	ret.m32 = a0 * rhs.m02 + a1 * rhs.m12 + a2 * rhs.m22 + a3 * rhs.m32;
	ret.m33 = a0 * rhs.m03 + a1 * rhs.m13 + a2 * rhs.m23 + a3 * rhs.m33;

#endif
}

template<typename T>
Mat44<T> Mat44<T>::operator*(const Mat44& rhs) const
{
	Mat44 result;
	mul(rhs, result);
	return result;
}

template<typename T>
Mat44<T>& Mat44<T>::operator*=(const Mat44& rhs)
{
	*this = *this * rhs;
	return *this;
}

template<typename T>
Mat44<T>& Mat44<T>::operator*=(const_param_type rhs) {
	return super_type::operator*=(rhs);
}

template<typename T>
void Mat44<T>::mul(const Vec4<T>& rhs, Vec4<T>& result) const
{
#if !defined(MCD_GCC) || defined(__SSE__)
	__m128 x0 = _mm_loadu_ps(c0);
	__m128 x1 = _mm_loadu_ps(c1);
	__m128 x2 = _mm_loadu_ps(c2);
	__m128 x3 = _mm_loadu_ps(c3);

	__m128 v = _mm_loadu_ps(rhs.getPtr());
	__m128 v0 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
	__m128 v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
	__m128 v2 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
	__m128 v3 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));

	v0 = _mm_mul_ps(x0, v0);
	v1 = _mm_mul_ps(x1, v1);
	v2 = _mm_mul_ps(x2, v2);
	v3 = _mm_mul_ps(x3, v3);

	x0 = _mm_add_ps(v0, v1);
	x1 = _mm_add_ps(v2, v3);
	v = _mm_add_ps(x0, x1);

	_mm_storeu_ps(result.getPtr(), v);

#else

	// Local variables to prevent parameter aliasing
	const float x = rhs.x;
	const float y = rhs.y;
	const float z = rhs.z;
	const float w = rhs.w;

	result.x = m00 * x + m01 * y + m02 * z + m03 * w;
	result.y = m10 * x + m11 * y + m12 * z + m13 * w;
	result.z = m20 * x + m21 * y + m22 * z + m23 * w;
	result.w = m30 * x + m31 * y + m32 * z + m33 * w;

#endif
}

template<typename T>
Vec4<T> Mat44<T>::operator*(const Vec4<T>& rhs) const
{
	Vec4<T> result;
	mul(rhs, result);
	return result;
}

template<typename T>
void Mat44<T>::transpose(Mat44& ret) const
{
	MCD_ASSERT(&ret != this);

	ret.m00 = m00; ret.m01 = m10; ret.m02 = m20; ret.m03 = m30;
	ret.m10 = m01; ret.m11 = m11; ret.m12 = m21; ret.m13 = m31;
	ret.m20 = m02; ret.m21 = m12; ret.m22 = m22; ret.m23 = m32;
	ret.m30 = m03; ret.m31 = m13; ret.m32 = m23; ret.m33 = m33;
}

template<typename T>
Mat44<T> Mat44<T>::transpose() const
{
	Mat44<T> result;
	transpose(result);
	return result;
}

template<typename T>
T Mat44<T>::determinant() const
{
	const T d12 = m20 * m31 - m30 * m21;
	const T d13 = m20 * m32 - m30 * m22;
	const T d23 = m21 * m32 - m31 * m22;
	const T d24 = m21 * m33 - m31 * m23;
	const T d34 = m22 * m33 - m32 * m23;
	const T d41 = m23 * m30 - m33 * m20;

	return
		m00 *  (m11 * d34 - m12 * d24 + m13 * d23) +
		m01 * -(m10 * d34 + m12 * d41 + m13 * d13) +
		m02 *  (m10 * d24 + m11 * d41 + m13 * d12) +
		m03 * -(m10 * d23 - m11 * d13 + m12 * d12);
}

template<typename T>
bool Mat44<T>::inverse(Mat44& result) const
{
	T v0 = m20 * m31 - m21 * m30;
	T v1 = m20 * m32 - m22 * m30;
	T v2 = m20 * m33 - m23 * m30;
	T v3 = m21 * m32 - m22 * m31;
	T v4 = m21 * m33 - m23 * m31;
	T v5 = m22 * m33 - m23 * m32;

	const T t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
	const T t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
	const T t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
	const T t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

	const T det = t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03;

	// We may got a very small determinant if the scaling part of the matrix is small
	if(Math<T>::isNearZero(det, 1e-20f))
		return false;

	const T invDet = T(1) / det;

	const T d00 = t00 * invDet;
	const T d10 = t10 * invDet;
	const T d20 = t20 * invDet;
	const T d30 = t30 * invDet;

	const T d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	const T d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	const T d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	const T d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	const T d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	const T d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	const T d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	const T d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	const T d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	const T d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	const T d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	const T d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	result.m00 = d00; result.m01 = d01; result.m02 = d02; result.m03 = d03;
	result.m10 = d10; result.m11 = d11; result.m12 = d12; result.m13 = d13;
	result.m20 = d20; result.m21 = d21; result.m22 = d22; result.m23 = d23;
	result.m30 = d30; result.m31 = d31; result.m32 = d32; result.m33 = d33;

	return true;
}

template<typename T>
Mat44<T> Mat44<T>::inverse() const
{
	Mat44 result;
	if(inverse(result))
		return result;
	return cIdentity;
}

template<typename T>
Vec3<T> Mat44<T>::xBiasVector() const {
	return Vec3<T>(m00, m10, m20);
}

template<typename T>
Vec3<T> Mat44<T>::yBiasVector() const {
	return Vec3<T>(m01, m11, m21);
}

template<typename T>
Vec3<T> Mat44<T>::zBiasVector() const {
	return Vec3<T>(m02, m12, m22);
}

template<typename T>
Vec3<T> Mat44<T>::translation() const
{
	return Vec3<T>(m03, m13, m23);
}

template<typename T>
void Mat44<T>::setTranslation(const Vec3<T>& translation)
{
	m03 = translation.x;
	m13 = translation.y;
	m23 = translation.z;
}

template<typename T>
void Mat44<T>::translateBy(const Vec3<T>& deltaTranslation)
{
	m03 += deltaTranslation.x;
	m13 += deltaTranslation.y;
	m23 += deltaTranslation.z;
}

template<typename T>
Vec3<T> Mat44<T>::scale() const
{
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=491578
	return Vec3<T>(
		Vec3<T>(m00, m10, m20).length() * (m00 > 0 ? 1 : -1.0f),
		Vec3<T>(m01, m11, m21).length() * (m11 > 0 ? 1 : -1.0f),
		Vec3<T>(m02, m12, m22).length() * (m22 > 0 ? 1 : -1.0f)
	);
}

template<typename T>
void Mat44<T>::setScale(const Vec3<T>& scale)
{
	const Vec3<T> currentScale = this->scale();

	MCD_ASSERT(scale[0] !=0 && scale[1] != 0 && scale[2] != 0);
	MCD_ASSERT(currentScale[0] !=0 && currentScale[1] != 0 && currentScale[2] != 0);

	// Scale the x, y and z bias vectors of the 3x3 matrix
	for(size_t i=0; i<3; ++i) {
		const T s = scale[i] / currentScale[i];
		for(size_t j=0; j<3; ++j)
			data2D[i][j] *= s;
	}
}

template<typename T>
void Mat44<T>::scaleBy(const Vec3<T>& deltaScale)
{
	MCD_ASSERT(deltaScale[0] != 0 && deltaScale[1] != 0 && deltaScale[2] != 0);

	// Scale the x, y and z bias vectors of the 3x3 matrix
	for(size_t i=0; i<3; ++i) {
		const T s = deltaScale[i];
		for(size_t j=0; j<3; ++j)
			data2D[i][j] *= s;
	}
}

template<typename T>
void Mat44<T>::setRotation(const Vec3<T>& axis, T angle)
{
	const Vec3<T> s = scale();
	const Vec3<T> t = translation();
	Mat33<T> tmp;
	*this = makeAxisRotation(axis, angle);
	scaleBy(s);
	translateBy(t);
}

template<typename T>
void Mat44<T>::rotateBy(const Vec3<T>& axis, T angle)
{
	const Vec3<T> t = translation();
	setTranslation(Vec3f::cZero);

	*this *= makeAxisRotation(axis, angle);

	setTranslation(t);
}

template<typename T>
void Mat44<T>::lookAt(const Vec3<T>& lookAt, const Vec3<T>& upVector)
{
	const Vec3<T> t = translation();
	const Vec3<T> sc = scale();

	const Vec3<T> f = (t - lookAt).normalizedCopy();
	const Vec3<T> s = f.cross(upVector.normalizedCopy());
	const Vec3<T> u = s.cross(f);

	m00 = s.x;	m01 = u.x;	m02 = -f.x;	m03 = t.x;
	m10 = s.y;	m11 = u.y;	m12 = -f.y;	m13 = t.y;
	m20 = s.z;	m21 = u.z;	m22 = -f.z;	m23 = t.z;
	m30 = 0;	m31 = 0;	m32 = 0;	m33 = 1.0f;

	setScale(sc);
}

template<typename T>
void Mat44<T>::lookAt(const Vec3<T>& eyeAt, const Vec3<T>& lookAt, const Vec3<T>& upVector)
{
	Vec3<T> f = (lookAt - eyeAt).normalizedCopy();
	Vec3<T> s = f.cross(upVector.normalizedCopy());
	Vec3<T> u = s.cross(f);

	m00 = s.x;	m01 = u.x;	m02 = -f.x;	m03 = 0;
	m10 = s.y;	m11 = u.y;	m12 = -f.y;	m13 = 0;
	m20 = s.z;	m21 = u.z;	m22 = -f.z;	m23 = 0;
	m30 = 0;	m31 = 0;	m32 = 0;	m33 = 1.0f;

	Mat44<T> tmp;
	tmp.copyFrom(data);
	tmp.setTranslation(eyeAt);
	tmp.copyTo(data);
}

template<typename T>
void Mat44<T>::mat33(Mat33<T>& matrix33) const
{
	matrix33.m00 = m00;	matrix33.m01 = m01;	matrix33.m02 = m02;
	matrix33.m10 = m10;	matrix33.m11 = m11;	matrix33.m12 = m12;
	matrix33.m20 = m20;	matrix33.m21 = m21;	matrix33.m22 = m22;
}

template<typename T>
Mat33<T> Mat44<T>::mat33() const
{
	Mat33<T> result;
	mat33(result);
	return result;
}

template<typename T>
void Mat44<T>::setMat33(const Mat33<T>& matrix33)
{
	m00 = matrix33.m00;	m01 = matrix33.m01;	m02 = matrix33.m02;
	m10 = matrix33.m10;	m11 = matrix33.m11;	m12 = matrix33.m12;
	m20 = matrix33.m20;	m21 = matrix33.m21;	m22 = matrix33.m22;
}

template<typename T>
void Mat44<T>::transformPoint(Vec3<T>& point) const
{
	Vec3<T> tmp = point;
	point.x = m00 * tmp.x + m01 * tmp.y + m02 * tmp.z + m03;
	point.y = m10 * tmp.x + m11 * tmp.y + m12 * tmp.z + m13;
	point.z = m20 * tmp.x + m21 * tmp.y + m22 * tmp.z + m23;
}

template<typename T>
void Mat44<T>::transformPointPerspective(Vec3<T>& point) const
{
	Vec4<T> tmp = *this * Vec4<T>(point, 1);
	T inv = T(1) / tmp.w;
	point.x = tmp.x * inv;
	point.y = tmp.y * inv;
	point.z = tmp.z * inv;
}

template<typename T>
void Mat44<T>::transformNormal(Vec3<T>& point) const
{
	Vec3<T> tmp = point;
	point.x = m00 * tmp.x + m01 * tmp.y + m02 * tmp.z;
	point.y = m10 * tmp.x + m11 * tmp.y + m12 * tmp.z;
	point.z = m20 * tmp.x + m21 * tmp.y + m22 * tmp.z;
}

template<typename T>
Mat44<T> Mat44<T>::makeScale(const Vec3<T>& scale) {
	Mat44<T> ret = cIdentity;
	ret.scaleBy(scale);
	return ret;
}

template<typename T>
Mat44<T> Mat44<T>::makeAxisRotation(const Vec3<T>& axis, T angle) {
	return Mat44f(Mat33f::makeAxisRotation(axis, angle));
}

template<typename T>
Mat44<T> Mat44<T>::makeTranslation(const Vec3<T>& translation) {
	Mat44<T> ret = cIdentity;
	ret.translateBy(translation);
	return ret;
}

template<typename T> const Mat44<T> Mat44<T>::cIdentity = Mat44<T>(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

}	// namespace MCD
