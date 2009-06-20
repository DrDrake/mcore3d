#include "../../../3Party/bullet/btBulletDynamicsCommon.h"

namespace MCD {

class RigidBodyComponent::Impl
{
public:
	Impl(float mass, CollisionShape* shape);

	~Impl();

	void onAttach(Entity* e);

	void update(Entity* e);

	btRigidBody* mRigidBody;
	btMotionState* mMotionState;
	CollisionShape* mShape;
	float mMass;
};	// Impl

}	// namespace MCD
