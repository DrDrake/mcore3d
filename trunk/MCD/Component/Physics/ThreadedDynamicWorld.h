#ifndef __MCD_COMPONENT_THREADEDDYNAMICWORLD__
#define __MCD_COMPONENT_THREADEDDYNAMICWORLD__

#include "../ShareLib.h"
#include "../../Core/System/Thread.h"
#include "../../Core/System/Mutex.h"

#include "DynamicsWorld.h"
#include <memory>
#include <queue>
#include <functional>

namespace MCD {

class MCD_COMPONENT_API ThreadedDynamicWorld : public Thread::IRunnable
{
private:
	std::auto_ptr<DynamicsWorld> mDynamicsWorld;
	// This is a prototype... Ignoring protability
	Mutex mCommandQueueLock;
	typedef std::queue<std::tr1::function<void()>> CommandQueue; // The current usage is simple enough to replace this queue by a lock-free queue
	CommandQueue mCommandQueue;
public:
	ThreadedDynamicWorld(void);
	virtual ~ThreadedDynamicWorld(void);

	virtual void run(Thread& thread) throw();

// ** Dynamic world functions start
// Operations
	void addRigidBody(RigidBodyComponent* rbc);

// Attributes
	void setGravity(const Vec3f& g);
// ** Dynamic world functions end
};

}  // namespace MCD
#endif // __MCD_COMPONENT_THREADEDDYNAMICWORLD__