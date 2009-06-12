#include "Pch.h"
#include "ShareLib.h"

#pragma warning( push )
#pragma warning( disable : 4100 )
#include "../3Party/bullet/btBulletDynamicsCommon.h"
#include "../3Party/bullet/btBulletCollisionCommon.h"
#pragma warning( pop ) 

#include "MathConvertor.h"

class btVector3;

namespace MCD
{
namespace BulletBinding
{
namespace MathConvertor
{
	MCD_BULLETBINDING_API btVector3 ToBullet(const MCD::Vec3f v)
	{
		return btVector3(v.x, v.y, v.z);
	};

	MCD_BULLETBINDING_API MCD::Vec3f ToMCD(const btVector3& v)
	{
		return MCD::Vec3f(v.x(), v.y(), v.z());
	};
}
}
}
