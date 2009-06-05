#include "Pch.h"
#include "RigidBodyComponent.h"

namespace MCD
{
namespace BulletBinding
{
	RigidBodyComponent::RigidBodyComponent(float mass, btCollisionShape* shape)
	{
		mShape.reset(shape);

		Entity* e = entity();

		// The quaternion is hardcoded.. fix later
		mMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(-1, 0, 0))));

		btRigidBody::btRigidBodyConstructionInfo rbInfo((btScalar)mass, mMotionState.get(), shape);

		mRigidBody.reset(new btRigidBody(rbInfo));
	}

	RigidBodyComponent::~RigidBodyComponent(void)
	{
		mRigidBody.reset();
		mMotionState.reset();
		mShape.reset();
	}

	void RigidBodyComponent::update()
	{
		Entity* e = entity();

		btTransform tx;
		mMotionState->getWorldTransform(tx);
		btMatrix3x3 rot = tx.getBasis();
		btVector3 v = tx.getOrigin();

		for(int x = 0; x < 3; x++)
			for(int y = 0; y < 3; y++)
				e->localTransform[x][y] = rot[x][y];

		e->localTransform.setTranslation(Vec3f(v[0], v[1], v[2]));
	}

}
}