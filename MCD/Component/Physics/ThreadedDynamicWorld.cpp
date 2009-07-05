#include "Pch.h"
#include "ThreadedDynamicWorld.h"

#include <iostream>

using namespace MCD;
using std::tr1::function;
using std::tr1::bind;

template<typename T>
struct functor_ret_voidifer : public std::unary_function<T, void>
{
	T mOp;

	functor_ret_voidifer(T op) : mOp(op)
	{
	};

	void operator()()
	{
		mOp();
	}
};

template<typename T>
function<void()> functor_ret_voidify(T op)
{
	return functor_ret_voidifer<T>(op);
}

void ThreadedDynamicWorld::run(Thread& thread) throw()
{
	while(thread.keepRun())
	{
		{
			ScopeLock cmdQueueLock(mCommandQueueLock);
			while(!mCommandQueue.empty())
			{
				mCommandQueue.front()();
				mCommandQueue.pop();
			}
		}
		mDynamicsWorld->stepSimulation(0.001f, 10);
	}
}

ThreadedDynamicWorld::ThreadedDynamicWorld(void) : mDynamicsWorld(new DynamicsWorld)
{
}

ThreadedDynamicWorld::~ThreadedDynamicWorld(void)
{
}

void ThreadedDynamicWorld::addRigidBody(RigidBodyComponent* rbc)
{
	ScopeLock cmdQueueLock(mCommandQueueLock);
	mCommandQueue.push(functor_ret_voidify(bind(&DynamicsWorld::addRigidBody, mDynamicsWorld.get(), rbc)));
}

void ThreadedDynamicWorld::setGravity(const Vec3f& g)
{
	ScopeLock cmdQueueLock(mCommandQueueLock);
	mCommandQueue.push(functor_ret_voidify(bind(&DynamicsWorld::setGravity, mDynamicsWorld.get(), g)));
}
