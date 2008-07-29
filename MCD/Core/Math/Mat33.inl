namespace MCD {

template<typename T>
const Vec3<T>& Mat33<T>::operator[](const size_t i) const
{
	MCD_ASSUME(i < rows());
	return *(reinterpret_cast<const Vec3<T>*>(&r0) + i);
}

template<typename T>
Vec3<T>& Mat33<T>::operator[](const size_t i)
{
	MCD_ASSUME(i < rows());
	return *(reinterpret_cast<Vec3<T>*>(&r0) + i);
}

template<typename T>
void Mat33<T>::mul(const Mat33& rhs, Mat33& result) const
{
	MCD_ASSUME(&rhs != &result);
	result.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20;
	result.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21;
	result.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22;
				 
	result.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20;
	result.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21;
	result.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22;
				 
	result.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20;
	result.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21;
	result.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22;
}

template<typename T>
Mat33<T> Mat33<T>::operator*(const Mat33& rhs) const
{
	Mat33 result;
	mul(rhs, result);
	return result;
}

template<typename T>
Mat33<T>& Mat33<T>::operator*=(const Mat33& rhs)
{
	*this = *this * rhs;
	return *this;
}

template<typename T>
Mat33<T>& Mat33<T>::operator*=(const param_type rhs) {
	return super_type::operator*=(rhs);
}

template<typename T>
void Mat33<T>::mul(const Vec3<T>& rhs, Vec3<T>& result) const
{
	MCD_ASSUME(&rhs != &result);
	result.x = m00 * rhs.x + m01 * rhs.y + m02 * rhs.z;
	result.y = m10 * rhs.x + m11 * rhs.y + m12 * rhs.z;
	result.z = m20 * rhs.x + m21 * rhs.y + m22 * rhs.z;
}

template<typename T>
Vec3<T> Mat33<T>::operator*(const Vec3<T>& rhs) const
{
	Vec3<T> result;
	mul(rhs, result);
	return result;
}

template<typename T>
void Mat33<T>::rotate(const param_type thetaX, const param_type thetaY, const param_type thetaZ, Mat33& result)
{
	T sinY, cosY;
	T sinX, cosX;
	T sinZ, cosZ;
	Math<T>::sinCos(thetaY, sinY, cosY);
	Math<T>::sinCos(thetaX, sinX, cosX);
	Math<T>::sinCos(thetaZ, sinZ, cosZ);

	result.m00 = cosY * cosZ + sinX * sinY * sinZ;
	result.m01 = cosZ * sinX * sinY - cosY * sinZ;
	result.m02 = cosX * sinY;

	result.m10 = cosX * sinZ;
	result.m11 = cosX * cosZ;
	result.m12 = -sinX;

	result.m20 = sinX * cosY * sinZ - sinY * cosZ;
	result.m21 = sinY * sinZ + sinX * cosY * cosZ;
	result.m22 = cosX * cosY;
}

template<typename T>
Mat33<T> Mat33<T>::rotate(const param_type thetaX, const param_type thetaY, const param_type thetaZ)
{
	Mat33 result;
	rotate(thetaX, thetaY, thetaZ, result);
	return result;
}

template<typename T> const Mat33<T> Mat33<T>::cIdentity = Mat33<T>(1, 0, 0, 0, 1, 0, 0, 0, 1);

}	// namespace MCD
