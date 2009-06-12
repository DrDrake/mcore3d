#ifndef __MCD_BULLETBINDING_MATHCONV__
#define __MCD_BULLETBINDING_MATHCONV__

#include "ShareLib.h"
#include "../MCD/Core/Math/Vec3.h"
class btVector3;

namespace MCD
{
namespace BulletBinding
{
namespace MathConvertor
{
	MCD_BULLETBINDING_API btVector3 ToBullet(const MCD::Vec3f v);
	MCD_BULLETBINDING_API MCD::Vec3f ToMCD(const btVector3& v);
}
}
}

#endif