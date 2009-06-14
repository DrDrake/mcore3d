#ifndef __MCD_BULLETBINDING_DYNAMICSWORLD__
#define __MCD_BULLETBINDING_DYNAMICSWORLD__

#include "../ShareLib.h"
#include "../../MCD/Core/Math/Vec3.h"
#include <memory>
#include <vector>

class btAxisSweep3;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btDynamicsWorld;
class btRigidBody;
class btSequentialImpulseConstraintSolver;

namespace MCD {

namespace BulletBinding {

class RigidBodyComponent;

class MCD_BULLETBINDING_API DynamicsWorld
{
public:
	DynamicsWorld(void);
	virtual ~DynamicsWorld(void);

	void setGravity(const Vec3f& g);
	Vec3f getGravity() const;

	void addRigidBody(RigidBodyComponent* rbc);
	void stepSimulation(float timeStep, int maxSubStep);

private:
	std::auto_ptr<btAxisSweep3> mBroadphase;
	std::auto_ptr<btDefaultCollisionConfiguration> mCollisionConfiguration;
	std::auto_ptr<btCollisionDispatcher> mDispatcher;
	std::auto_ptr<btSequentialImpulseConstraintSolver> mSolver;
	std::auto_ptr<btDynamicsWorld> mDynamicsWorld;

	std::vector<btRigidBody*> mRigidBodies;
};	// DynamicsWorld

}	// BulletBinding

}	// MCD

#endif	// __MCD_BULLETBINDING_DYNAMICSWORLD__
