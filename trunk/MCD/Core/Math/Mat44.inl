#if !defined(MCD_GCC) || defined(__SSE__)
#   include <xmmintrin.h>
#endif

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
	MCD_ASSUME(i < rows());
	return *(reinterpret_cast<const Vec4<T>*>(&r0) + i);
}

template<typename T>
Vec4<T>& Mat44<T>::operator[](const size_t i)
{
	MCD_ASSUME(i < rows());
	return *(reinterpret_cast<Vec4<T>*>(&r0) + i);
}

template<typename T>
void Mat44<T>::mul(const Mat44& rhs, Mat44& ret) const
{
	MCD_ASSUME(&rhs != &ret);
	MCD_ASSUME(this != &ret);

#if !defined(MCD_GCC) || defined(__SSE__)
	__m128 x4 = _mm_loadu_ps(rhs.r0);
	__m128 x5 = _mm_loadu_ps(rhs.r1);
	__m128 x6 = _mm_loadu_ps(rhs.r2);
	__m128 x7 = _mm_loadu_ps(rhs.r3);

	__m128 x0, x1, x2, x3;

	for(size_t i=0; i<4; ++i) {
		x1 = x2 = x3 = x0 = _mm_loadu_ps(this->data2D[i]);
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

	// Seems using "restrict" optimization in this function didn't reduce memory movement, but
	// more code is generated.
	ret.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30;
	ret.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31;
	ret.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32;
	ret.m03 = m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33;

	ret.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30;
	ret.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31;
	ret.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32;
	ret.m13 = m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33;

	ret.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30;
	ret.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31;
	ret.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32;
	ret.m23 = m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33;

	ret.m30 = m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30;
	ret.m31 = m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31;
	ret.m32 = m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32;
	ret.m33 = m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33;

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
Mat44<T>& Mat44<T>::operator*=(const param_type rhs) {
	return super_type::operator*=(rhs);
}

template<typename T>
void Mat44<T>::mul(const Vec4<T>& rhs_, Vec4<T>& result) const
{
	Vec4<T>* __restrict ret = &result;
	const Vec4<T>* __restrict rhs = &rhs_;
	MCD_ASSUME(rhs != ret);

	ret->x = m00 * rhs->x + m01 * rhs->y + m02 * rhs->z + m03 * rhs->w;
	ret->y = m10 * rhs->x + m11 * rhs->y + m12 * rhs->z + m13 * rhs->w;
	ret->z = m20 * rhs->x + m21 * rhs->y + m22 * rhs->z + m23 * rhs->w;
	ret->w = m30 * rhs->x + m31 * rhs->y + m32 * rhs->z + m33 * rhs->w;
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

	if(Math<T>::isNearZero(det))
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
Vec3<T> Mat44<T>::scale() const
{
	return Vec3<T>(m00, m11, m22);
}

template<typename T>
void Mat44<T>::setScale(const Vec3<T>& scale)
{
	m00 = scale.x;
	m11 = scale.y;
	m22 = scale.z;
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
void Mat44<T>::transformNormal(Vec3<T>& point) const
{
	Vec3<T> tmp = point;
	point.x = m00 * tmp.x + m01 * tmp.y + m02 * tmp.z;
	point.y = m10 * tmp.x + m11 * tmp.y + m12 * tmp.z;
	point.z = m20 * tmp.x + m21 * tmp.y + m22 * tmp.z;
}

template<typename T> const Mat44<T> Mat44<T>::cIdentity = Mat44<T>(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

}	// namespace MCD
