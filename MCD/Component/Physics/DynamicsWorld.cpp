#include "Pch.h"
#include "DynamicsWorld.h"
#include "DynamicsWorld.inl"
#include "RigidBodyComponent.h"
#include "RigidBodyComponent.inl"	// We need to access some implementation of RigidBodyComponent
#include "MathConvertor.inl"
#include "../../../3Party/bullet/btBulletDynamicsCommon.h"

using namespace MCD;

#ifdef MCD_VC
#	ifdef NDEBUG
#		pragma comment(lib, "libbulletdynamics")
#		pragma comment(lib, "libbulletcollision")
#		pragma comment(lib, "libbulletmath")
#	else
#		pragma comment(lib, "libbulletdynamicsd")
#		pragma comment(lib, "libbulletcollisiond")
#		pragma comment(lib, "libbulletmathd")
#	endif
#endif

DynamicsWorld::Impl::Impl()
{
	btVector3 worldAabbMin(-1000,-1000,-1000);
	btVector3 worldAabbMax( 1000, 1000, 1000);

	const unsigned short maxProxies = 1500;

	// Create the btDiscreteDynamicsWorld
	// The world configuation is temporary hardcoded
	mBroadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);
	mCollisionConfiguration = new btDefaultCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
	mSolver = new btSequentialImpulseConstraintSolver();

	mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
}

DynamicsWorld::Impl::~Impl()
{
	// NOTE: Must delete the object in order
	delete mDynamicsWorld;
	delete mSolver;
	delete mDispatcher;
	delete mCollisionConfiguration;
	delete mBroadphase;
}

DynamicsWorld::DynamicsWorld()
{
	mImpl = new Impl();
}

DynamicsWorld::~DynamicsWorld()
{
	delete mImpl;
}

void DynamicsWorld::setGravity(const Vec3f& g)
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	mImpl->mDynamicsWorld->setGravity(toBullet(g));
}

Vec3f DynamicsWorld::gravity() const
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	return toMCD(mImpl->mDynamicsWorld->getGravity());
}

void DynamicsWorld::stepSimulation(float timeStep, int maxSubStep)
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	mImpl->mDynamicsWorld->stepSimulation(timeStep, maxSubStep);
}

void DynamicsWorld::addRigidBody(RigidBodyComponent& rbc)
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	// NOTE: rbc.mImpl may be null, when using with ThreadedDynamicsWorld
	if(rbc.mImpl)
		mImpl->mDynamicsWorld->addRigidBody(rbc.mImpl->mRigidBody);
}

void DynamicsWorld::removeRigidBody(RigidBodyComponent& rbc)
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	// NOTE: rbc.mImpl may be null, when using with ThreadedDynamicsWorld
	if(rbc.mImpl) {
		// NOTE: If you saw memory error on the next line, most likely you haven't
		// make sure all RigidBodyComponent are destroyed before the DynamicsWorld destroy.
		mImpl->mDynamicsWorld->removeRigidBody(rbc.mImpl->mRigidBody);
	}
}

void DynamicsWorld::removeRigidBody(void* rbc)
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	btRigidBody* p = reinterpret_cast<btRigidBody*>(rbc);
	// NOTE: If you saw memory error on the next line, most likely you haven't
	// make sure all RigidBodyComponent are destroyed before the DynamicsWorld destroy.
	mImpl->mDynamicsWorld->removeRigidBody(p);
}
