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
bool Mat33<T>::getRotationXYZ(T& thetaX, T& thetaY, T& thetaZ)
{
	thetaY = Math<T>::aSinClamp(m02);
	if(thetaY < Math<T>::cPiOver2())
	{
		if(thetaY > -Math<T>::cPiOver2())
		{
			thetaX = ::atan2(-m12, m22);
			thetaZ = ::atan2(-m01, m00);
			return true;
		}
		else
		{
			// Not an unique solution.
			T rmY = ::atan2(m10, m11);
			thetaZ = 0;	// Any angle works
			thetaX = thetaZ - rmY;
			return false;
		}
	}
	else
	{
		// Not an unique solution.
		T rpY = ::atan2(m10, m11);
		thetaZ = 0;	// Any angle works
		thetaX = rpY - thetaZ;
		return false;
	}
}

template<typename T>
void Mat33<T>::rotateXYZ(const param_type thetaX, const param_type thetaY, const param_type thetaZ, Mat33& result)
{
	// Reference: OgreMatrix3.cpp
	float cos, sin;

	Math<T>::sinCos(thetaX, sin, cos);
	const Mat33 xMat(1, 0, 0, 0, cos, -sin, 0, sin, cos);

	Math<T>::sinCos(thetaY, sin, cos);
	const Mat33 yMat(cos, 0, sin, 0, 1, 0, -sin, 0, cos);

	Math<T>::sinCos(thetaZ, sin, cos);
	const Mat33 zMat(cos, -sin, 0, sin, cos, 0, 0, 0, 1);

	result = xMat * (yMat * zMat);
}

template<typename T>
Mat33<T> Mat33<T>::rotateXYZ(const param_type thetaX, const param_type thetaY, const param_type thetaZ)
{
	Mat33 result;
	rotateXYZ(thetaX, thetaY, thetaZ, result);
	return result;
}

template<typename T>
void Mat33<T>::rotate(const Vec3<T>& axis, T angle, Mat33& result)
{
	// Reference: OgreMatrix3.cpp
	T cos, sin;
	Math<T>::sinCos(angle, sin, cos);

	T oneMinusCos = 1 - cos;
	T x2 = axis.x * axis.x;
	T y2 = axis.y * axis.y;
	T z2 = axis.z * axis.z;
	T xym = axis.x * axis.y * oneMinusCos;
	T xzm = axis.x * axis.z * oneMinusCos;
	T yzm = axis.y * axis.z * oneMinusCos;
	T xSin = axis.x * sin;
	T ySin = axis.y * sin;
	T zSin = axis.z * sin;

	result.m00 = x2 * oneMinusCos + cos;
	result.m01 = xym - zSin;
	result.m02 = xzm + ySin;
	result.m10 = xym + zSin;
	result.m11 = y2 * oneMinusCos + cos;
	result.m12 = yzm - xSin;
	result.m20 = xzm - ySin;
	result.m21 = yzm + xSin;
	result.m22 = z2 * oneMinusCos + cos;
}

template<typename T>
Mat33<T> Mat33<T>::rotate(const Vec3<T>& axis, T angle)
{
	Mat33 result;
	rotate(axis, angle, result);
	return result;
}

template<typename T> const Mat33<T> Mat33<T>::cIdentity = Mat33<T>(1, 0, 0, 0, 1, 0, 0, 0, 1);

}	// namespace MCD
