#include "BasicFunction.h"

namespace MCD {

template<typename T>
Quaternion<T> Quaternion<T>::inverse() const
{
	const T len2 = squaredLength();
	if(len2 > 0) {
		const T invLen2 = T(1) / len2;
		return conjugate() * invLen2;
	} else {
		// Return an invalid result to flag the error
		return Quaternion(0, 0, 0, 0);
	}
}

template<typename T>
Quaternion<T> Quaternion<T>::inverseUnit() const
{
	MCD_ASSERT(Math<T>::isNearEqual(1, squaredLength()));
	return conjugate();
}

template<typename T>
void Quaternion<T>::mul(const Quaternion& rhs, Quaternion& result) const
{
	MCD_ASSUME(&rhs != &result);
	result.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
	result.y = w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z;
	result.z = w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x;
	result.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Quaternion& rhs) const
{
	Quaternion result;
	mul(rhs, result);
	return result;
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator*=(const Quaternion& rhs)
{
	*this = *this * rhs;
	return *this;
}

template<typename T>
Quaternion<T>& Quaternion<T>::fromAxisAngle(const Vec3<T>& axis, param_type angle)
{
	T sin, cos;
	Math<T>::sinCos(angle * T(0.5), sin, cos);

	const T axisLen2 = axis.squaredLength();
	MCD_ASSUME(axisLen2 > 0);
	const T scaleDown = axisLen2 > 0 ? T(sqrt(axisLen2)) : 1;
	const Vec3<T> u(axis * (sin / scaleDown));
	setVec3(u);
	w = cos;

	return *this;
}

template<typename T>
void Quaternion<T>::toAxisAngle(Vec3<T>& axis, T& angle) const
{
	const T axisLen2 = getVec3().squaredLength();

	if(axisLen2 > 0) {
		axis = getVec3() * (T(1) / T(sqrt(axisLen2)));
		angle = Math<T>::aCosClamp(w) * 2;
	} else {
		// Angle is 0, so any axis will do
		axis = Vec3<T>(0, 0, 1);
		angle = 0;
	}
}

template<typename T>
Quaternion<T>& Quaternion<T>::fromMatrix(const Mat33<T>& matrix)
{
	MCD_ASSERT(Math<T>::isNearEqual(matrix.determinant(), 1) && "The matrix should be orthoginal");
	MCD_ASSERT(matrix.m00 + matrix.m11 + matrix.m22 + 1 > 0 && "The matrix should be special orthoginal");

	// Reference: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
	// The max(0, ...) is just a safeguard against rounding error.
	// TODO: A very good fit for using SIMD :)
	x = (T)sqrt(Math<T>::max(0, 1 + matrix.m00 - matrix.m11 - matrix.m22)) * T(0.5);
	y = (T)sqrt(Math<T>::max(0, 1 - matrix.m00 + matrix.m11 - matrix.m22)) * T(0.5);
	z = (T)sqrt(Math<T>::max(0, 1 - matrix.m00 - matrix.m11 + matrix.m22)) * T(0.5);
	w = (T)sqrt(Math<T>::max(0, 1 + matrix.m00 + matrix.m11 + matrix.m22)) * T(0.5);

#ifdef MCD_VC
#	define copysign _copysign
#endif
	x = (T)copysign(x, matrix.m21 - matrix.m12);
	y = (T)copysign(y, matrix.m02 - matrix.m20);
	z = (T)copysign(z, matrix.m10 - matrix.m01);

	return *this;
}

template<typename T>
void Quaternion<T>::toMatrix(Mat33<T>& matrix) const
{
	// Reference: OgreQuaternion.cpp
	const T tx  = 2 * x;
	const T ty  = 2 * y;
	const T tz  = 2 * z;
	const T twx = tx * w;
	const T twy = ty * w;
	const T twz = tz * w;
	const T txx = tx * x;
	const T txy = ty * x;
	const T txz = tz * x;
	const T tyy = ty * y;
	const T tyz = tz * y;
	const T tzz = tz * z;

	matrix.m00 = 1 - (tyy + tzz);
	matrix.m01 = txy - twz;
	matrix.m02 = txz + twy;
	matrix.m10 = txy + twz;
	matrix.m11 = 1 - (txx + tzz);
	matrix.m12 = tyz - twx;
	matrix.m20 = txz - twy;
	matrix.m21 = tyz + twx;
	matrix.m22 = 1 - (txx + tyy);
}

template<typename T>
Mat33<T> Quaternion<T>::toMatrix() const
{
	Mat33<T> result;
	toMatrix(result);
	return result;
}

template<typename T>
void Quaternion<T>::transform(Vec3<T>& v) const
{
	// Reference: http://www.gamedev.net/reference/articles/article1824.asp
	v = (*this * Quaternion(v, 1) * inverseUnit()).getVec3();
}

template<typename T>
bool Quaternion<T>::isNearEqual(const Quaternion<T>& rhs, T tolerance) const
{
	return
		Math<T>::isNearEqual(x, rhs.x, tolerance) &&
		Math<T>::isNearEqual(y, rhs.y, tolerance) &&
		Math<T>::isNearEqual(z, rhs.z, tolerance) &&
		Math<T>::isNearEqual(w, rhs.w, tolerance);
}

template<typename T>
Quaternion<T> Quaternion<T>::slerp(const Quaternion& q1, const Quaternion& q2, T t)
{
	// Refernece: From ID software, "Slerping Clock Cycles"
	const T cosVal = q1 % q2;
	const T absCosVal = fabsf(cosVal);
	if((1.0f - absCosVal) > 1e-6f)
	{
		// Standard case (slerp)
		const T sinSqr = 1.0f - absCosVal * absCosVal;
		const T invSin = 1.0f / sqrtf(sinSqr);
		const T omega = Mathf::aTanPositive(sinSqr * invSin, absCosVal);
		const T scale0 = sinf((1.0f - t) * omega) * invSin;
		T scale1 = sinf(t * omega) * invSin;

		scale1 = (cosVal >= 0.0f) ? scale1 : -scale1;

		return scale0 * q1 + scale1 * q2;
	}
	else	// Fallback to linear
		return q1 + t * (q2 - q1);
}

template<typename T> const Quaternion<T> Quaternion<T>::cIdentity = Quaternion(0, 0, 0, 1);

}	// namespace MCD
