#ifndef __MCD_CORE_MATH_MAGNITUDE__
#define __MCD_CORE_MATH_MAGNITUDE__

#include "../System/Platform.h"

namespace MCD {

/*!	Stores a squared value of a magnitude.
	Useful when a function returns a magnitude like Vec3::length()
	so that comparing the result to a floating point value is optimal.
	For example:
	\code
	Vec3f v(49);
	if(v.Legth() < 8) {
		// ...
	}
	\endcode

	is equivalence to:
	\code
	Vec3f v(49);
	if(v.squaredLength() < 8*8) {
		// ...
	}
	\endcode
 */
template<typename T>
class Magnitude
{
public:
	explicit Magnitude(T squaredMagnitude)
		: mSquaredMagnitude(squaredMagnitude)
	{
	}

	T GetMagnitude() const {
		return ::sqrt(mSquaredMagnitude);
	}

	operator T() const {
		return GetMagnitude();
	}

	template<typename U>
	bool operator==(U val) const {
		return mSquaredMagnitude == val * val;
	}

	template<typename U>
	bool operator!=(U val) const {
		return mSquaredMagnitude != val * val;
	}

	template<typename U>
	bool operator<(U val) const {
		return mSquaredMagnitude < val * val;
	}

	template<typename U>
	bool operator<=(U val) const {
		return mSquaredMagnitude <= val * val;
	}

	template<typename U>
	bool operator>(U val) const {
		return mSquaredMagnitude > val * val;
	}

	template<typename U>
	bool operator>=(U val) const {
		return mSquaredMagnitude >= val * val;
	}

protected:
	T mSquaredMagnitude;
};	// Magnitude

}	// namespace MCD

#endif	// __MCD_CORE_MATH_MAGNITUDE__
