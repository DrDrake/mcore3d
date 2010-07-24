#include "../../../3Party/bullet/btBulletDynamicsCommon.h"

namespace MCD {

class DynamicsWorld;

class RigidBodyComponent::Impl
{
public:
	Impl(DynamicsWorld& dynamicsWorld, float mass, const CollisionShapePtr& shape);

	~Impl();

	void onAdd(sal_in Entity* e);

	void update(sal_in Entity* e);

	DynamicsWorld* mDynamicsWorld;
	btRigidBody* mRigidBody;
	btMotionState* mMotionState;
	CollisionShapePtr mShape;
	float mMass;
};	// Impl

}	// namespace MCD
