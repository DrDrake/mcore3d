#ifndef __MCD_CORE_MATH_SRTTRANSFORM__
#define __MCD_CORE_MATH_SRTTRANSFORM__

#include "Mat44.h"
#include "Quaternion.h"
#include "../ShareLib.h"

namespace MCD {

/*!	A 4 by 4 matrix.
 */
class MCD_CORE_API SrtTransform
{
public:
	SrtTransform(const Mat44f& matrix);

// Operation

// Attribute
	Quaternionf rotation;
	Vec3f translation;
	Vec3f scale;
};	// SrtTransform

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SRTTRANSFORM__
