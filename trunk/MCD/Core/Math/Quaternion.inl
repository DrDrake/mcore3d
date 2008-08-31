namespace MCD {

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
void Quaternion<T>::fromAxisAngle(const Vec3<T>& axis, float angle)
{
	T sin, cos;
	Math<T>::sinCos(angle * T(0.5), sin, cos);

	Vec3<T> u(axis.normalizedCopy() * sin);
	setVec3(u);
	w = cos;
}

}	// namespace MCD
