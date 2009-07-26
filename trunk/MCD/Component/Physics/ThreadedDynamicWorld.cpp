#include "Pch.h"
#include "ThreadedDynamicWorld.h"
#include "DynamicsWorld.inl"
#include "RigidBodyComponent.h"
#include "RigidBodyComponent.inl"
#include "../../Core/Entity/Component.h"
#include "../../Core/System/Mutex.h"
#include "../../Core/System/Timer.h"
#include "../../Core/System/TypeTrait.h"
#include <queue>

//! A simply enough command class for use in physics component command queue.
class MCD_ABSTRACT_CLASS ICommand {
public:
	virtual ~ICommand() {}
	virtual void exec() = 0;
};	// ICommand

template<typename T1, typename T2, typename T1_=ParamType<T1>::RET, typename T2_=ParamType<T2>::RET>
struct StaticCommand2 : public ICommand
{
	typedef void (*F)(T1_, T2_);
	StaticCommand2(F f, T1_ t1, T2_ t2) : f(f), t1(t1), t2(t2) {}
	sal_override void exec() { (*f)(t1, t2); }

	F f;
	T1 t1;	// The storage type can be differ from the argument type.
	T2 t2;
};	// StaticCommand2

template<class C, typename T1, typename T1_=ParamType<T1>::RET>
struct MemCommand1 : public ICommand
{
	typedef void (C::*F)(T1_);
	MemCommand1(F f, C& c, T1_ t1) : f(f), c(c), t1(t1) {}
	sal_override void exec() { (c.*f)(t1); }

	F f;
	C& c;
	T1 t1;	// The storage type can be differ from the argument type.
};	// MemCommand1

using namespace MCD;

class ThreadedDynamicsWorld::Impl
{
public:
	Impl(ThreadedDynamicsWorld& world) : mThreadedDynamicsWorld(world) {}

	~Impl()
	{
		// Cleanup all pending job before closing, some job may be removing
		// btRigidBody from btDynamicsWorld which is essential during shutdown.
		doQueueJob();
	}

	void doQueueJob() 
	{
		ScopeLock lock(mCommandQueueLock);
		while(!mCommandQueue.empty())
		{
			ICommand* c = mCommandQueue.front();
			c->exec();
			delete c;
			mCommandQueue.pop();
		}
	}

	void run(Thread& thread) throw()
	{
		DeltaTimer timer;
		while(thread.keepRun())
		{
			doQueueJob();

			float dt = float(timer.getDelta().asSecond());
			mThreadedDynamicsWorld.stepSimulation(dt, 10);

			float instanceFps = 1.0f / dt;

			// Limit the framerate of the physics thread
			if(instanceFps > cFpsLimit)
				mSleep(size_t(1000.0f / instanceFps));
		}

		doQueueJob();
	}

	ThreadedDynamicsWorld& mThreadedDynamicsWorld;
	Mutex mCommandQueueLock;
	typedef std::queue<ICommand*> CommandQueue;
	CommandQueue mCommandQueue;

	static const int cFpsLimit = 30;
};	// Impl

void ThreadedDynamicsWorld::run(Thread& thread) throw()
{
	MCD_ASSUME(mImpl);
	mImpl->run(thread);
}

ThreadedDynamicsWorld::ThreadedDynamicsWorld()
{
	mImpl = new Impl(*this);
}

ThreadedDynamicsWorld::~ThreadedDynamicsWorld()
{
	delete mImpl;
}

void ThreadedDynamicsWorld::addRigidBody(RigidBodyComponent& rbc)
{
	struct Dummy
	{
		// Note that we use ComponentPtr (a weak pointer) as parameter
		static void addRigidBody(ThreadedDynamicsWorld& world, const ComponentPtr& rbc)
		{
			RigidBodyComponent* p = dynamic_cast<RigidBodyComponent*>(rbc.get());
			if(p)
				world.addRigidBodyNoQueue(*p);
		}
	};	// Dummy

	MCD_ASSUME(mImpl);
	ScopeLock lock(mImpl->mCommandQueueLock);
	mImpl->mCommandQueue.push(new StaticCommand2<ThreadedDynamicsWorld&, const ComponentPtr>(&Dummy::addRigidBody, *this, ComponentPtr(&rbc)));
}

void ThreadedDynamicsWorld::addRigidBodyNoQueue(RigidBodyComponent& rbc)
{
	MCD_ASSUME(mImpl);
	MCD_ASSERT(mImpl->mCommandQueueLock.isLocked());
	DynamicsWorld::addRigidBody(rbc);
}

void ThreadedDynamicsWorld::setGravity(const Vec3f& g)
{
	MCD_ASSUME(mImpl);
	ScopeLock lock(mImpl->mCommandQueueLock);
	mImpl->mCommandQueue.push(new MemCommand1<DynamicsWorld, const Vec3f>(&DynamicsWorld::setGravity, *this, g));
}

void ThreadedDynamicsWorld::removeRigidBody(RigidBodyComponent& rbc)
{
	struct Dummy
	{
		// Note that we use ComponentPtr (a weak pointer) as parameter
		static void removeRigidBody(ThreadedDynamicsWorld& world, RigidBodyComponent::Impl* rbc)
		{
			if(rbc->mRigidBody)
				world.removeRigidBodyNoQueue(rbc->mRigidBody);
			delete rbc;
		}
	};	// Dummy

	// Remove the ownership of btRigidBody from RigidBodyComponent to the command.
	MCD_ASSUME(rbc.mImpl);
	RigidBodyComponent::Impl* p = rbc.mImpl;
	rbc.mImpl = nullptr;

	MCD_ASSUME(mImpl);
	ScopeLock lock(mImpl->mCommandQueueLock);
	mImpl->mCommandQueue.push(new StaticCommand2<ThreadedDynamicsWorld&, RigidBodyComponent::Impl*>(&Dummy::removeRigidBody, *this, p));
}

void ThreadedDynamicsWorld::removeRigidBodyNoQueue(void* rbc)
{
	DynamicsWorld::removeRigidBody(rbc);
}
