#include "Pch.h"
#include "MathConvertor.h"

#include "../3Party/bullet/LinearMath/btVector3.h"

#ifdef NDEBUG
#	pragma comment(lib, "libbulletdynamics")
#	pragma comment(lib, "libbulletcollision")
#	pragma comment(lib, "libbulletmath")
#else
#	pragma comment(lib, "libbulletdynamicsd")
#	pragma comment(lib, "libbulletcollisiond")
#	pragma comment(lib, "libbulletmathd")
#endif

namespace MCD {
namespace BulletBinding {
namespace MathConvertor {

btVector3 ToBullet(const Vec3f& v)
{
	return btVector3(v.x, v.y, v.z);
}

Vec3f ToMCD(const btVector3& v)
{
	return MCD::Vec3f(v.x(), v.y(), v.z());
}

}	// MathConvertor
}	// BulletBinding
}	// MCD
