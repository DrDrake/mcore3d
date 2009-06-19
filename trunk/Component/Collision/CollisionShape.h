#ifndef __MCD_BULLETBINDING_COLLISIONSHAPE__
#define __MCD_BULLETBINDING_COLLISIONSHAPE__

#include "../ShareLib.h"
#include "../../MCD/Core/Math/Vec3.h"
#include <memory>
#include "../../MCD/Core/System/SharedPtr.h"

class btCollisionShape;

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;

namespace BulletBinding {

class MCD_BULLETBINDING_API CollisionShape
{
protected:
	std::auto_ptr<btCollisionShape> mShape;
public:
	CollisionShape(void) {};
	CollisionShape(btCollisionShape* shape);
	virtual ~CollisionShape(void) {};
	btCollisionShape* getCollisionShape() const{
		return mShape.get();
	};
};  // CollisionShape

class MCD_BULLETBINDING_API SphereShape : public CollisionShape
{
public:
	SphereShape(float radius);
	virtual ~SphereShape(void) {};
};

class MCD_BULLETBINDING_API StaticPlaneShape : public CollisionShape
{
public:
	StaticPlaneShape(const Vec3f& planeNormal, float planeConstant);
	virtual ~StaticPlaneShape(void) {};
};

class MCD_BULLETBINDING_API TriMeshShape : public CollisionShape
{
public:
	TriMeshShape(const MeshPtr& mesh);
	virtual ~TriMeshShape(void) {};
};

}	// BulletBinding

}	// MCD

#endif // __MCD_BULLETBINDING_COLLISIONSHAPE__