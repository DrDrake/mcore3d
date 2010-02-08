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
Mat33<T>& Mat33<T>::operator*=(const_param_type rhs) {
	return super_type::operator*=(rhs);
}

template<typename T>
void Mat33<T>::mul(const Vec3<T>& rhs, Vec3<T>& result) const
{
	// Local variables to prevent parameter aliasing
	const float x = rhs.x;
	const float y = rhs.y;
	const float z = rhs.z;

	result.x = m00 * x + m01 * y + m02 * z;
	result.y = m10 * x + m11 * y + m12 * z;
	result.z = m20 * x + m21 * y + m22 * z;
}

template<typename T>
Vec3<T> Mat33<T>::operator*(const Vec3<T>& rhs) const
{
	Vec3<T> result;
	mul(rhs, result);
	return result;
}

template<typename T>
void Mat33<T>::transpose(Mat33& ret) const
{
	MCD_ASSERT(&ret != this);

	ret.m00 = m00; ret.m01 = m10; ret.m02 = m20;
	ret.m10 = m01; ret.m11 = m11; ret.m12 = m21;
	ret.m20 = m02; ret.m21 = m12; ret.m22 = m22;
}

template<typename T>
Mat33<T> Mat33<T>::transpose() const
{
	Mat33<T> result;
	transpose(result);
	return result;
}

template<typename T>
T Mat33<T>::determinant() const
{
	return
		m00 * (m11 * m22 - m12 * m21) -
		m01 * (m10 * m22 - m12 * m20) +
		m02 * (m10 * m21 - m11 * m20);
}

template<typename T>
Vec3<T> Mat33<T>::scale() const
{
	// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=491578
	return Vec3<T>(
		Vec3<T>(m00, m10, m20).length(),
		Vec3<T>(m01, m11, m21).length(),
		Vec3<T>(m02, m12, m22).length()
	);
}

template<typename T>
void Mat33<T>::setScale(const Vec3<T>& scale)
{
	const Vec3<T> currentScale = this->scale();

	MCD_ASSERT(scale[0] != 0 && scale[1] != 0 && scale[2] != 0);
	MCD_ASSERT(currentScale[0] !=0 && currentScale[1] != 0 && currentScale[2] != 0);

	// Scale the x, y and z bias vectors of the 3x3 matrix
	for(size_t i=0; i<3; ++i) {
		const T s = scale[i] / currentScale[i];
		for(size_t j=0; j<3; ++j)
			data2D[j][i] *= s;
	}
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
			const T rmY = ::atan2(m10, m11);
			thetaZ = 0;	// Any angle works
			thetaX = thetaZ - rmY;
			return false;
		}
	}
	else
	{
		// Not an unique solution.
		const T rpY = ::atan2(m10, m11);
		thetaZ = 0;	// Any angle works
		thetaX = rpY - thetaZ;
		return false;
	}
}

template<typename T>
void Mat33<T>::makeXYZRotation(const_param_type thetaX, const_param_type thetaY, const_param_type thetaZ, Mat33& result)
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
Mat33<T> Mat33<T>::makeXYZRotation(const_param_type thetaX, const_param_type thetaY, const_param_type thetaZ)
{
	Mat33 result;
	makeXYZRotation(thetaX, thetaY, thetaZ, result);
	return result;
}

template<typename T>
void Mat33<T>::makeAxisRotation(const Vec3<T>& axis, T angle, Mat33& result)
{
	// Reference: OgreMatrix3.cpp
	T cos, sin;
	Math<T>::sinCos(angle, sin, cos);

	const T oneMinusCos = 1 - cos;
	const T x2 = axis.x * axis.x;
	const T y2 = axis.y * axis.y;
	const T z2 = axis.z * axis.z;
	const T xym = axis.x * axis.y * oneMinusCos;
	const T xzm = axis.x * axis.z * oneMinusCos;
	const T yzm = axis.y * axis.z * oneMinusCos;
	const T xSin = axis.x * sin;
	const T ySin = axis.y * sin;
	const T zSin = axis.z * sin;

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
Mat33<T> Mat33<T>::makeAxisRotation(const Vec3<T>& axis, T angle)
{
	Mat33 result;
	makeAxisRotation(axis, angle, result);
	return result;
}

template<typename T> const Mat33<T> Mat33<T>::cIdentity = Mat33<T>(1, 0, 0, 0, 1, 0, 0, 0, 1);

}	// namespace MCD
