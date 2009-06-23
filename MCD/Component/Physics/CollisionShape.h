#ifndef __MCD_COMPONENT_COLLISIONSHAPE__
#define __MCD_COMPONENT_COLLISIONSHAPE__

#include "../ShareLib.h"
#include "../../Core/System/NonCopyable.h"
#include "../../Core/System/SharedPtr.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

typedef IntrusivePtr<class Mesh> MeshPtr;

/*!
	The ownership of a CollisionShape is shared by multiple instance of RigidBodyComponent.
 */
class MCD_ABSTRACT_CLASS MCD_COMPONENT_API CollisionShape : Noncopyable
{
	friend class RigidBodyComponent;

protected:
	CollisionShape();

	//! Please make sure the shape variable is kind of btCollisionShape
	CollisionShape(void* shape);

	virtual ~CollisionShape();

	/*!	Pointer storing the implementation of the collision shape,
		that is actually using the type btCollisionShape.
	 */
	void* shapeImpl;

	// Function required for intrusive pointer 
	MCD_COMPONENT_API friend void intrusivePtrAddRef(CollisionShape* p);

	MCD_COMPONENT_API friend void intrusivePtrRelease(CollisionShape* p);

	AtomicInteger mRefCount;
};  // CollisionShape

typedef IntrusivePtr<CollisionShape> CollisionShapePtr;

class MCD_COMPONENT_API SphereShape : public CollisionShape
{
public:
	SphereShape(float radius);
};	// SphereShape

class MCD_COMPONENT_API StaticPlaneShape : public CollisionShape
{
public:
	StaticPlaneShape(const Vec3f& planeNormal, float planeConstant);
};	// StaticPlaneShape

class MCD_COMPONENT_API StaticTriMeshShape : public CollisionShape
{
public:
	StaticTriMeshShape(const MeshPtr& mesh);

	sal_override ~StaticTriMeshShape();

private:
	class Impl;
	Impl* mImpl;
};	// StaticTriMeshShape

}	// MCD

#endif // __MCD_COMPONENT_COLLISIONSHAPE__