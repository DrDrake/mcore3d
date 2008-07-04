#ifndef __SGE_CORE_MATH_BASICFUNCTION__
#define __SGE_CORE_MATH_BASICFUNCTION__

#include "../System/Platform.h"
#include <math.h>

namespace SGE {

template<class Real>
class Math
{
public:
	//! The base of the natural logarithms.
	static Real cE() { return static_cast<Real>(2.71828182845904523536); }

	//! Log2(e)
	static Real cLog2E() { return static_cast<Real>(1.44269504088896340736); }

	//! Log10(e)
	static Real cLog10E() { return static_cast<Real>(0.434294481903251827651); }

	//! Ln(2)
	static Real cLn2() { return static_cast<Real>(0.693147180559945309417); }

	//! Ln(10)
	static Real cLn10() { return static_cast<Real>(2.30258509299404568402); }

	//! The Pi value 3.1415...
	static Real cPi() { return static_cast<Real>(3.14159265358979323846); }

	//! Pi / 2
	static Real cPiOver2() { return static_cast<Real>(1.57079632679489661923); }

	//! Pi / 4
	static Real cPiOver4() { return static_cast<Real>(0.785398163397448309616); }

	//! 1 / Pi
	static Real cOverPi() { return static_cast<Real>(0.318309886183790671538); }

	//! 2 / Pi
	static Real c2OverPi() { return static_cast<Real>(0.636619772367581343076); }

	//! 2 / Sqrt(Pi)
	static Real c2OverSqrtPi() { return static_cast<Real>(1.12837916709551257390); }

	//! Sqrt(2)
	static Real cSqrt2() { return static_cast<Real>(1.41421356237309504880); }

	//! 1 / Sqrt(2)
	static Real cOverSqrt2() { return static_cast<Real>(0.707106781186547524401); }
};	// Math

}	// namespace SGE

#endif	// __SGE_CORE_MATH_BASICFUNCTION__
