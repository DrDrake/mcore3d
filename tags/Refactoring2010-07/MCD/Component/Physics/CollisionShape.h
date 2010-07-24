#ifndef __MCD_COMPONENT_COLLISIONSHAPE__
#define __MCD_COMPONENT_COLLISIONSHAPE__

#include "../ShareLib.h"
#include "../../Core/System/NonCopyable.h"
#include "../../Core/System/ScriptOwnershipHandle.h"
#include "../../Core/System/SharedPtr.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

typedef IntrusivePtr<class Mesh> MeshPtr;

class MeshBuilder;

/*!
	The ownership of a CollisionShape is shared by multiple instance of RigidBodyComponent.
 */
class MCD_ABSTRACT_CLASS MCD_COMPONENT_API CollisionShape : public IntrusiveSharedObject<AtomicInteger>, Noncopyable
{
	friend class RigidBodyComponent;

public:
	ScriptOwnershipHandle scriptOwnershipHandle;

protected:
	CollisionShape();

	//! Please make sure the shape variable is kind of btCollisionShape
	CollisionShape(void* shape);

	virtual ~CollisionShape();

	virtual bool isStatic() const = 0;

	/*!	Pointer storing the implementation of the collision shape,
		that is actually using the type btCollisionShape.
	 */
	void* shapeImpl;
};  // CollisionShape

typedef IntrusivePtr<CollisionShape> CollisionShapePtr;

class MCD_COMPONENT_API SphereShape : public CollisionShape
{
public:
	SphereShape(float radius);

	sal_override bool isStatic() const { return false; }

protected:
	sal_override ~SphereShape() {}
};	// SphereShape

typedef IntrusivePtr<SphereShape> SphereShapePtr;

class MCD_COMPONENT_API StaticPlaneShape : public CollisionShape
{
public:
	StaticPlaneShape(const Vec3f& planeNormal, float planeConstant);

	sal_override bool isStatic() const { return true; }

protected:
	sal_override ~StaticPlaneShape() {}
};	// StaticPlaneShape

typedef IntrusivePtr<StaticPlaneShape> StaticPlaneShapePtr;

class MCD_COMPONENT_API StaticTriMeshShape : public CollisionShape
{
public:
	StaticTriMeshShape(const MeshPtr& mesh);

	/*!	Create StaticTriMeshShape from a MeshBuilder.
		\note If you knows the mesh builder will not be destroyed before the StaticTriMeshShape,
			you can pass keepOwnBuffer = false, use with care!
	 */
	StaticTriMeshShape(const MeshBuilder& meshBuilder, int positionId, bool keepOwnBuffer=true);

	sal_override bool isStatic() const { return true; }

protected:
	sal_override ~StaticTriMeshShape();

private:
	class Impl;
	Impl& mImpl;
};	// StaticTriMeshShape

typedef IntrusivePtr<StaticTriMeshShape> StaticTriMeshShapePtr;

}	// MCD

#endif // __MCD_COMPONENT_COLLISIONSHAPE__