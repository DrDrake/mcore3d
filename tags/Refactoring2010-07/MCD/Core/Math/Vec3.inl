#include "BasicFunction.h"

namespace MCD {

template<typename T>
T Vec3<T>::dot(const Vec3& rhs) const {
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

template<typename T>
T Vec3<T>::operator%(const Vec3& rhs) const {
	return dot(rhs);
}

template<typename T>
T Vec3<T>::norm() const {
	return dot(*this);
}

template<typename T>
void Vec3<T>::cross(const Vec3& rhs, Vec3& result) const
{
	result.x = y * rhs.z - z * rhs.y;
	result.y = z * rhs.x - x * rhs.z;
	result.z = x * rhs.y - y * rhs.x;
}

template<typename T>
Vec3<T> Vec3<T>::cross(const Vec3& rhs) const
{
	Vec3 result;
	cross(rhs, result);
	return result;
}

template<typename T>
Vec3<T> Vec3<T>::operator^(const Vec3& rhs) const {
	return cross(rhs);
}

template<typename T>
Magnitude<T> Vec3<T>::length() const {
	return Magnitude<param_type>(squaredLength());
}

template<typename T>
T Vec3<T>::squaredLength() const {
	return dot(*this);
}

template<typename T>
T Vec3<T>::distance(const Vec3& rhs) const {
	return (*this - rhs).length();
}

template<typename T>
T Vec3<T>::squaredDistance(const Vec3& rhs) const {
	return (*this - rhs).squaredLength();
}

template<typename T>
T Vec3<T>::normalize()
{
	T len = length();
	(*this) *= (T(1)/length());
	return len;
}

template<typename T>
T Vec3<T>::normalizeSafe()
{
	T len = length();
	if(len > 1e-8)
		(*this) *= (T(1)/length());
	return len;
}

template<typename T>
Vec3<T> Vec3<T>::normalizedCopy() const {
	return (*this) * (T(1)/length());
}

template<typename T> const Vec3<T> Vec3<T>::cZero = Vec3(0);
template<typename T> const Vec3<T> Vec3<T>::c100 = Vec3(1, 0, 0);
template<typename T> const Vec3<T> Vec3<T>::c010 = Vec3(0, 1, 0);
template<typename T> const Vec3<T> Vec3<T>::c001 = Vec3(0, 0, 1);

}	// namespace MCD
