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
void Mat33<T>::mul(const Mat33& rhs_, Mat33& result) const
{
	Mat33* __restrict ret = &result;
	const Mat33* __restrict lhs = this;
	const Mat33* __restrict rhs = &rhs_;
	MCD_ASSUME(rhs != ret);
	MCD_ASSUME(lhs != ret);

	ret->m00 = lhs->m00 * rhs->m00 + lhs->m01 * rhs->m10 + lhs->m02 * rhs->m20;
	ret->m01 = lhs->m00 * rhs->m01 + lhs->m01 * rhs->m11 + lhs->m02 * rhs->m21;
	ret->m02 = lhs->m00 * rhs->m02 + lhs->m01 * rhs->m12 + lhs->m02 * rhs->m22;
															  
	ret->m10 = lhs->m10 * rhs->m00 + lhs->m11 * rhs->m10 + lhs->m12 * rhs->m20;
	ret->m11 = lhs->m10 * rhs->m01 + lhs->m11 * rhs->m11 + lhs->m12 * rhs->m21;
	ret->m12 = lhs->m10 * rhs->m02 + lhs->m11 * rhs->m12 + lhs->m12 * rhs->m22;
															  
	ret->m20 = lhs->m20 * rhs->m00 + lhs->m21 * rhs->m10 + lhs->m22 * rhs->m20;
	ret->m21 = lhs->m20 * rhs->m01 + lhs->m21 * rhs->m11 + lhs->m22 * rhs->m21;
	ret->m22 = lhs->m20 * rhs->m02 + lhs->m21 * rhs->m12 + lhs->m22 * rhs->m22;
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
void Mat33<T>::mul(const Vec3<T>& rhs_, Vec3<T>& result) const
{
	Vec3<T>* __restrict ret = &result;
	const Vec3<T>* __restrict rhs = &rhs_;
	MCD_ASSUME(rhs != ret);

	ret->x = m00 * rhs->x + m01 * rhs->y + m02 * rhs->z;
	ret->y = m10 * rhs->x + m11 * rhs->y + m12 * rhs->z;
	ret->z = m20 * rhs->x + m21 * rhs->y + m22 * rhs->z;
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
	T sinX, cosX;
	T sinY, cosY;
	T sinZ, cosZ;
	Math<T>::sinCos(thetaX, sinX, cosX);
	Math<T>::sinCos(thetaY, sinY, cosY);
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
