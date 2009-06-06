#include "Pch.h"
#include "DynamicsWorld.h"
#include "RigidBodyComponent.h"

using namespace MCD::BulletBinding;

DynamicsWorld::DynamicsWorld(void)
{
	btVector3 worldAabbMin(-10000,-10000,-10000);
	btVector3 worldAabbMax( 10000, 10000, 10000);

	int maxProxies = 1024;

	// Create the btDiscreteDynamicsWorld
	// The world configuation is temporary hardcoded
	mBroadphase.reset(new btAxisSweep3(worldAabbMin, worldAabbMax, (unsigned short)maxProxies));
	mCollisionConfiguration.reset(new btDefaultCollisionConfiguration());
	mDispatcher.reset(new btCollisionDispatcher(mCollisionConfiguration.get()));
	mSolver.reset(new btSequentialImpulseConstraintSolver);

	mDynamicsWorld.reset(new btDiscreteDynamicsWorld(mDispatcher.get(), mBroadphase.get(), mSolver.get(), mCollisionConfiguration.get()));
}

DynamicsWorld::~DynamicsWorld(void)
{
	for(int i = 0; i < mRigidBodies.size(); i++)
		mDynamicsWorld->removeRigidBody(mRigidBodies[i]);

	// Free the memory in order
	mDynamicsWorld.reset();
	mSolver.reset();
	mDispatcher.reset();
	mCollisionConfiguration.reset();
	mBroadphase.reset();
}

void DynamicsWorld::setGravity(MCD::Vec3f g)
{
	mDynamicsWorld->setGravity(MathConvertor::ToBullet(g));
}

MCD::Vec3f DynamicsWorld::getGravity() const
{
	return MathConvertor::ToMCD(mDynamicsWorld->getGravity());
}

void DynamicsWorld::addRigidBody(RigidBodyComponent* rbc)
{
	mRigidBodies.push_back(rbc->getRightBody());
	mDynamicsWorld->addRigidBody(rbc->getRightBody());
}

void DynamicsWorld::stepSimulation(float timeStep, int maxSubStep)
{
	mDynamicsWorld->stepSimulation(timeStep, maxSubStep);
}
