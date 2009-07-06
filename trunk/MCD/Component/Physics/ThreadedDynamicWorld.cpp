#include "Pch.h"
#include "ThreadedDynamicWorld.h"
#include "RigidBodyComponent.h"
#include "../../Core/System/Mutex.h"
#include "../../Core/System/Timer.h"
#include "../../Core/Entity/Component.h"
#include <functional>
#include <queue>

using namespace MCD;
using std::tr1::bind;
using std::tr1::function;

template<typename T>
struct functor_ret_voidifer : public std::unary_function<T, void>
{
	functor_ret_voidifer(T op) : mOp(op) {}

	void operator()() {
		mOp();
	}

	T mOp;
};	// functor_ret_voidifer

template<typename T>
function<void()> functor_ret_voidify(T op)
{
	return functor_ret_voidifer<T>(op);
}

// Note that we use ComponentPtr (a weak pointer) as parameter
static void addRigidBody(DynamicsWorld* world, const ComponentPtr& rbc)
{
	MCD_ASSUME(world);
	RigidBodyComponent* p = dynamic_cast<RigidBodyComponent*>(rbc.get());
	if(p)
		world->addRigidBody(*p);
}

class ThreadedDynamicWorld::Impl : public DynamicsWorld
{
public:
	Impl() : mSmoothFps(0) {}

	void run(Thread& thread) throw()
	{
		DeltaTimer timer;
		while(thread.keepRun())
		{
			{
				ScopeLock lock(mCommandQueueLock);
				while(!mCommandQueue.empty())
				{
					mCommandQueue.front()();
					mCommandQueue.pop();
				}
			}

			float dt = float(timer.getDelta().asSecond());
			stepSimulation(dt, 10);

			float instanceFps = 1.0f / dt;
			mSmoothFps = instanceFps / 10 + mSmoothFps / 9;

			// Limit the framerate of the physics thread
			if(mSmoothFps > cFpsLimit)
				mSleep(size_t(1000.0f / mSmoothFps));
		}
	}

	void addRigidBody(RigidBodyComponent& rbc)
	{
		ScopeLock lock(mCommandQueueLock);
		mCommandQueue.push(functor_ret_voidify(bind(&::addRigidBody, this, ComponentPtr(&rbc))));
	}

	void setGravity(const Vec3f& g)
	{
		ScopeLock lock(mCommandQueueLock);
		mCommandQueue.push(functor_ret_voidify(bind(&DynamicsWorld::setGravity, this, g)));
	}

	Mutex mCommandQueueLock;
	typedef std::queue<std::tr1::function<void()> > CommandQueue;
	CommandQueue mCommandQueue;
	float mSmoothFps;	// A smoothed frame per second

	static const int cFpsLimit = 30;
};	// Impl

void ThreadedDynamicWorld::run(Thread& thread) throw()
{
	MCD_ASSUME(mImpl);
	mImpl->run(thread);
}

ThreadedDynamicWorld::ThreadedDynamicWorld()
{
	mImpl = new Impl();
}

ThreadedDynamicWorld::~ThreadedDynamicWorld()
{
	delete mImpl;
}

void ThreadedDynamicWorld::addRigidBody(RigidBodyComponent& rbc)
{
	MCD_ASSUME(mImpl);
	mImpl->addRigidBody(rbc);
}

void ThreadedDynamicWorld::setGravity(const Vec3f& g)
{
	MCD_ASSUME(mImpl);
	mImpl->setGravity(g);
}
