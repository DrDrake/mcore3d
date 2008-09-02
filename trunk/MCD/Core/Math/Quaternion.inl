#include "BasicFunction.h"

namespace MCD {

template<typename T>
Quaternion<T> Quaternion<T>::inverse() const
{
	T len2 = squaredLength();
	if(len2 > 0) {
		T invLen2 = T(1) / len2;
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
void Quaternion<T>::fromAxisAngle(const Vec3<T>& axis, T angle)
{
	T sin, cos;
	Math<T>::sinCos(angle * T(0.5), sin, cos);

	T axisLen2 = axis.squaredLength();
	MCD_ASSUME(axisLen2 > 0);
	T scaleDown = axisLen2 > 0 ? sqrt(axisLen2) : 1;
	Vec3<T> u(axis * (sin / scaleDown));
	setVec3(u);
	w = cos;
}

template<typename T>
void Quaternion<T>::toAxisAngle(Vec3<T>& axis, T& angle) const
{
	T axisLen2 = getVec3().squaredLength();

	if(axisLen2 > 0) {
		axis = getVec3() * (T(1) / sqrt(axisLen2));
		angle = Math<T>::aCosClamp(w) * 2;
	} else {
		// Angle is 0, so any axis will do
		axis = Vec3<T>(0, 0, 1);
		angle = 0;
	}
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

template<typename T> const Quaternion<T> Quaternion<T>::cIdentity = Quaternion(0, 0, 0, 1);

}	// namespace MCD
