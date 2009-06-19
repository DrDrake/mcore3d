#ifndef __MCD_BULLETBINDING_COLLISIONSHAPE__
#define __MCD_BULLETBINDING_COLLISIONSHAPE__

#include "../ShareLib.h"
#include "../../MCD/Core/System/NonCopyable.h"
#include "../../MCD/Core/System/SharedPtr.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

typedef IntrusivePtr<class Mesh> MeshPtr;

namespace BulletBinding {

class MCD_ABSTRACT_CLASS MCD_BULLETBINDING_API CollisionShape : Noncopyable
{
	friend class RigidBodyComponent;

protected:
	CollisionShape();

	//! Please make sure the shape variable is kind of btCollisionShape
	CollisionShape(void* shape);

	~CollisionShape();

	/*!	Pointer storing the implementation of the collision shape,
		that is actually using the type btCollisionShape.
	 */
	void* shapeImpl;
};  // CollisionShape

class MCD_BULLETBINDING_API SphereShape : public CollisionShape
{
public:
	SphereShape(float radius);
};	// SphereShape

class MCD_BULLETBINDING_API StaticPlaneShape : public CollisionShape
{
public:
	StaticPlaneShape(const Vec3f& planeNormal, float planeConstant);
};	// StaticPlaneShape

class MCD_BULLETBINDING_API TriMeshShape : public CollisionShape
{
public:
	TriMeshShape(const MeshPtr& mesh);
};	// TriMeshShape

}	// BulletBinding

}	// MCD

#endif // __MCD_BULLETBINDING_COLLISIONSHAPE__