/*!	This header is usefull if you need to access the internal
	functionarity of physics compoent, and need to work with
	bullet physics engine directly.
 */

#include "../MCD/Core/Math/Vec3.h"
#include "../3Party/bullet/LinearMath/btVector3.h"

namespace MCD {

namespace BulletBinding {

/*!	We cannot use reinterpret_cast when converting from Vec3f to btVector3,
	because of some potential memory size and allignment issues.
 */
inline btVector3 toBullet(const Vec3f& v) {
	return btVector3(v.x, v.y, v.z);
}

inline const Vec3f& toMCD(const btVector3& v) {
	return reinterpret_cast<const Vec3f&>(v);
}

}	// BulletBinding

}	// MCD
