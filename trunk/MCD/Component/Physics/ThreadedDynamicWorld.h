#ifndef __MCD_COMPONENT_THREADEDDYNAMICWORLD__
#define __MCD_COMPONENT_THREADEDDYNAMICWORLD__

#include "../ShareLib.h"
#include "../../Core/System/Thread.h"

#include "DynamicsWorld.h"

namespace MCD {

class MCD_COMPONENT_API ThreadedDynamicWorld : public Thread::IRunnable
{
public:
	ThreadedDynamicWorld(void);

	sal_override ~ThreadedDynamicWorld();

	sal_override void run(Thread& thread) throw();

// Operations
	//! ThreadedDynamicWorld will not take over the ownership of RigidBodyComponent
	// TODO: Add the corresponding remove function.
	void addRigidBody(RigidBodyComponent& rbc);

// Attributes
	void setGravity(const Vec3f& g);

private:
	class Impl;
	Impl* mImpl;
};	// ThreadedDynamicWorld

}  // namespace MCD

#endif	// __MCD_COMPONENT_THREADEDDYNAMICWORLD__
