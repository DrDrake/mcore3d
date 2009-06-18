#include "Pch.h"
#include "RigidBodyComponent.h"
#include "../MathConvertor.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../3Party/bullet/btBulletDynamicsCommon.h"

namespace MCD {

namespace BulletBinding {

RigidBodyComponent::RigidBodyComponent(float mass, CollisionShape* shape) : mMass(mass)
{
	mShape = shape;

	//Entity* e = entity();

	// The btRigidBody should not be created now.. It should be created when the component is attached to an entity
	// The quaternion is hardcoded.. fix later
	// mMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 0, 0))));
}

RigidBodyComponent::~RigidBodyComponent(void)
{
	// Destructs the object in sequence...
	mRigidBody.reset();
	mMotionState.reset();
}

void RigidBodyComponent::onAttach()
{
	Entity* e = entity();
	
	btTransform tx(btQuaternion(0,0,0,1), MathConvertor::ToBullet(e->localTransform.translation()));

	mMotionState.reset(new btDefaultMotionState(tx));

	btRigidBody::btRigidBodyConstructionInfo rbInfo((btScalar)mMass, mMotionState.get(), mShape->getCollisionShape());

	mRigidBody.reset(new btRigidBody(rbInfo));
}

void RigidBodyComponent::update()
{
	// Simple translation of btTransform to MCD Matrix
	Entity* e = entity();

	btTransform tx;
	mMotionState->getWorldTransform(tx);
	const btMatrix3x3 rot(tx.getBasis());
	const btVector3 v(tx.getOrigin());

	for(int x = 0; x < 3; ++x)
		for(int y = 0; y < 3; ++y)
			e->localTransform[x][y] = rot[x][y];

	e->localTransform.setTranslation(Vec3f(v[0], v[1], v[2]));
}

void RigidBodyComponent::activate()
{
	mRigidBody->activate();
}

void RigidBodyComponent::applyForce(const Vec3f& force, const Vec3f& rel_pos)
{
	using namespace MathConvertor;
	mRigidBody->applyForce(ToBullet(force), ToBullet(rel_pos));
}

float RigidBodyComponent::getLinearDamping() const
{
	return mRigidBody->getLinearDamping();
}

float RigidBodyComponent::getAngularDamping() const
{
	return mRigidBody->getAngularDamping();
}

void RigidBodyComponent::setDamping(float lin_damping, float ang_damping)
{
	mRigidBody->setDamping(lin_damping, ang_damping);
}

}	// BulletBinding

}	// MCD
