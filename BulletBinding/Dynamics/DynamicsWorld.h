#ifndef __MCD_BULLETBINDING_DYNAMICSWORLD__
#define __MCD_BULLETBINDING_DYNAMICSWORLD__

#include <memory>
#include <vector>

namespace MCD
{
namespace BulletBinding
{
	class RigidBodyComponent;
	class DynamicsWorld
	{
	private:
		std::auto_ptr<btAxisSweep3> mBroadphase;
		std::auto_ptr<btDefaultCollisionConfiguration> mCollisionConfiguration;
		std::auto_ptr<btCollisionDispatcher> mDispatcher;
		std::auto_ptr<btSequentialImpulseConstraintSolver> mSolver;
		std::auto_ptr<btDynamicsWorld> mDynamicsWorld;

		std::vector<btRigidBody*> mRigidBodies;
	public:
		DynamicsWorld(void);
		virtual ~DynamicsWorld(void);

		void setGravity(Vec3f g);
		Vec3f getGravity() const;

		void addRigidBody(RigidBodyComponent* rbc);
		void stepSimulation(float timeStep, int maxSubStep);
	};
}
}

#endif