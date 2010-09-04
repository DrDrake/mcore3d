#ifndef __MCD_COMPONENT_ThreadedDynamicsWorld__
#define __MCD_COMPONENT_ThreadedDynamicsWorld__

#include "../ShareLib.h"
#include "../../Core/System/Thread.h"

#include "DynamicsWorld.h"

namespace MCD {

class MCD_COMPONENT_API ThreadedDynamicsWorld : public DynamicsWorld, public Thread::IRunnable
{
public:
	ThreadedDynamicsWorld(void);

	sal_override ~ThreadedDynamicsWorld();

	sal_override void run(Thread& thread);

// Operations

// Attributes
	void setGravity(const Vec3f& g);

protected:
	//! ThreadedDynamicsWorld will not take over the ownership of RigidBodyComponent
	sal_override void addRigidBody(RigidBodyComponent& rbc);
	void addRigidBodyNoQueue(RigidBodyComponent& rbc);

	sal_override void removeRigidBody(RigidBodyComponent& rbc);

	//! Expecting void* to be btRigidBody*
	void removeRigidBodyNoQueue(sal_notnull void* rbc);

private:
	class Impl;
	Impl* mImpl;
};	// ThreadedDynamicsWorld

}  // namespace MCD

#endif	// __MCD_COMPONENT_ThreadedDynamicsWorld__
