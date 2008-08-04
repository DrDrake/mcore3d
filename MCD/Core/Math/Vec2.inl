#include "BasicFunction.h"

namespace MCD {

template<typename T>
T Vec2<T>::dot(const Vec2& rhs) const {
	return x * rhs.x + y * rhs.y;
}

template<typename T>
T Vec2<T>::operator%(const Vec2& rhs) const {
	return dot(rhs);
}

template<typename T>
Magnitude<T> Vec2<T>::length() const {
	return Magnitude<param_type>(squaredLength());
}

template<typename T>
T Vec2<T>::squaredLength() const {
	return dot(*this);
}

template<typename T>
T Vec2<T>::distance(const Vec2& rhs) const {
	return (*this - rhs).length();
}

template<typename T>
T Vec2<T>::squaredDistance(const Vec2& rhs) const {
	return (*this - rhs).squaredLength();
}

template<typename T>
T Vec2<T>::normalize()
{
	T len = length();
	(*this) *= (T(1)/length());
	return len;
}

template<typename T>
T Vec2<T>::normalizeSafe()
{
	T len = length();
	if(len > 1e-8)
		(*this) *= (T(1)/length());
	return len;
}

template<typename T>
Vec2<T> Vec2<T>::normalizedCopy() const {
	return (*this) * (T(1)/length());
}

template<typename T>
bool Vec2<T>::isNearEqual(const Vec2& rhs, T tolerance) const
{
	return
		Math<T>::isNearEqual(x, rhs.x, tolerance) &&
		Math<T>::isNearEqual(y, rhs.y, tolerance);
}

template<typename T> const Vec2<T> Vec2<T>::cZero = Vec2(0);
template<typename T> const Vec2<T> Vec2<T>::c10 = Vec2(1, 0);
template<typename T> const Vec2<T> Vec2<T>::c01 = Vec2(0, 1);

}	// namespace MCD
