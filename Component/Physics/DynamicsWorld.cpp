#include "Pch.h"
#include "DynamicsWorld.h"
#include "RigidBodyComponent.h"
#include "RigidBodyComponent.inl"	// We need to access some implementation of RigidBodyComponent
#include "MathConvertor.inl"
#include "../../MCD/Core/System/PtrVector.h"
#include "../../3Party/bullet/btBulletDynamicsCommon.h"

using namespace MCD;
using namespace MCD::PhysicsComponent;

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

class DynamicsWorld::Impl
{
public:
	Impl()
	{
		btVector3 worldAabbMin(-100,-100,-100);
		btVector3 worldAabbMax( 100, 100, 100);

		const unsigned short maxProxies = 1024;

		// Create the btDiscreteDynamicsWorld
		// The world configuation is temporary hardcoded
		mBroadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);
		mCollisionConfiguration = new btDefaultCollisionConfiguration();
		mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
		mSolver = new btSequentialImpulseConstraintSolver();

		mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
	}

	~Impl()
	{
		// Remove the rigid bodies in the dynamics world, but don't delete them,
		// since it's ownership is owned by RigidBodyComponent
		for(RigidBodies::size_type i=0; i < mRigidBodies.size(); ++i)
			mDynamicsWorld->removeRigidBody(mRigidBodies[i]);

		// NOTE: Must delete the object in order
		delete mDynamicsWorld;
		delete mSolver;
		delete mDispatcher;
		delete mCollisionConfiguration;
		delete mBroadphase;
	}

	btAxisSweep3* mBroadphase;
	btDefaultCollisionConfiguration* mCollisionConfiguration;
	btCollisionDispatcher* mDispatcher;
	btSequentialImpulseConstraintSolver* mSolver;
	btDynamicsWorld* mDynamicsWorld;

	// Only weak reference, not owning them.
	// TODO: Seems there is no need to store btRigidBody, remove it
	typedef std::vector<btRigidBody*> RigidBodies;
	RigidBodies mRigidBodies;
};	// Impl

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
	MCD_ASSUME(mImpl->mDynamicsWorld);
	mImpl->mDynamicsWorld->setGravity(toBullet(g));
}

Vec3f DynamicsWorld::gravity() const
{
	MCD_ASSUME(mImpl->mDynamicsWorld);
	return toMCD(mImpl->mDynamicsWorld->getGravity());
}

void DynamicsWorld::addRigidBody(RigidBodyComponent* rbc)
{
	MCD_ASSUME(mImpl);
	MCD_ASSUME(mImpl->mDynamicsWorld);
	mImpl->mRigidBodies.push_back(rbc->mImpl->mRigidBody);
	mImpl->mDynamicsWorld->addRigidBody(rbc->mImpl->mRigidBody);
}

void DynamicsWorld::stepSimulation(float timeStep, int maxSubStep)
{
	MCD_ASSUME(mImpl->mDynamicsWorld);
	mImpl->mDynamicsWorld->stepSimulation(timeStep, maxSubStep);
}
