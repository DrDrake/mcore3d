#ifndef __MCD_BULLETBINDING_MATHCONV__
#define __MCD_BULLETBINDING_MATHCONV__

namespace MCD
{
namespace BulletBinding
{
	struct MathConvertor
	{
		static btVector3 ToBullet(const MCD::Vec3f v)
		{
			return btVector3(v.x, v.y, v.z);
		};

		static MCD::Vec3f ToMCD(const btVector3& v)
		{
			return MCD::Vec3f(v.x(), v.y(), v.z());
		};
	};
}
}

#endif