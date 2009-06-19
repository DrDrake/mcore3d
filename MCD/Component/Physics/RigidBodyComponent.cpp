#include "Pch.h"
#include "RigidBodyComponent.h"
#include "RigidBodyComponent.inl"
#include "CollisionShape.h"
#include "MathConvertor.inl"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/bullet/btBulletDynamicsCommon.h"

namespace MCD {

namespace PhysicsComponent {

RigidBodyComponent::Impl::Impl(float mass, CollisionShape* shape)
{
	mMass = mass;
	mShape = shape;

	//Entity* e = entity();

	// The btRigidBody should not be created now.. It should be created when the component is attached to an entity
	// The quaternion is hardcoded.. fix later
	// mMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0))));
}

RigidBodyComponent::Impl::~Impl()
{
	// Destructs the object in sequence...
	delete mRigidBody;
	delete mMotionState;
}

void RigidBodyComponent::Impl::onAttach(Entity* e)
{
	btTransform tx(btQuaternion(0,0,0,1), toBullet(e->localTransform.translation()));

	mMotionState = new btDefaultMotionState(tx);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		(btScalar)mMass, mMotionState, reinterpret_cast<btCollisionShape*>(mShape->shapeImpl)
	);

	mRigidBody = new btRigidBody(rbInfo);
}

void RigidBodyComponent::Impl::update(Entity* e)
{
	// Simple translation of btTransform to MCD Matrix
	btTransform tx;
	mMotionState->getWorldTransform(tx);
	const btMatrix3x3 rot(tx.getBasis());
	const btVector3 v(tx.getOrigin());

	for(int x = 0; x < 3; ++x)
		for(int y = 0; y < 3; ++y)
			e->localTransform[x][y] = rot[x][y];

	e->localTransform.setTranslation(Vec3f(v[0], v[1], v[2]));
}

RigidBodyComponent::RigidBodyComponent(float mass, CollisionShape* shape)
{
	mImpl = new Impl(mass, shape);
}

RigidBodyComponent::~RigidBodyComponent(void)
{
	delete mImpl;
}

void RigidBodyComponent::onAttach()
{
	MCD_ASSUME(mImpl);
	mImpl->onAttach(entity());
}

void RigidBodyComponent::update()
{
	MCD_ASSUME(mImpl);
	mImpl->update(entity());
}

void RigidBodyComponent::activate()
{
	MCD_ASSUME(mImpl);
	mImpl->mRigidBody->activate();
}

void RigidBodyComponent::applyForce(const Vec3f& force, const Vec3f& rel_pos)
{
	MCD_ASSUME(mImpl);
	mImpl->mRigidBody->applyForce(toBullet(force), toBullet(rel_pos));
}

float RigidBodyComponent::getLinearDamping() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mRigidBody->getLinearDamping();
}

float RigidBodyComponent::getAngularDamping() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mRigidBody->getAngularDamping();
}

void RigidBodyComponent::setDamping(float lin_damping, float ang_damping)
{
	MCD_ASSUME(mImpl);
	mImpl->mRigidBody->setDamping(lin_damping, ang_damping);
}

}	// namespace PhysicsComponent

}	// namespace MCD
