#include "Pch.h"
#include "RigidBodyComponent.h"
#include "RigidBodyComponent.inl"
#include "CollisionShape.h"
#include "DynamicsWorld.h"
#include "DynamicsWorld.inl"
#include "MathConvertor.inl"
#include "../Render/MeshComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/bullet/btBulletDynamicsCommon.h"

namespace MCD {

RigidBodyComponent::Impl::Impl(DynamicsWorld& dynamicsWorld, float mass, const CollisionShapePtr& shape)
	: mDynamicsWorld(&dynamicsWorld), mRigidBody(nullptr), mMotionState(nullptr), mShape(shape), mMass(mass)
{
}

RigidBodyComponent::Impl::~Impl()
{
	// Destructs the object in sequence...
	delete mRigidBody;
	delete mMotionState;
}

void RigidBodyComponent::Impl::onAdd(Entity* e)
{
	// TODO: Add some warning if the incomming Entity didn't have an identity wrold transform.
	MCD_ASSUME(e);

	btTransform tx(btQuaternion(0,0,0,1), toBullet(e->localTransform.translation()));

	mMotionState = new btDefaultMotionState(tx);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		(btScalar)mMass, mMotionState, reinterpret_cast<btCollisionShape*>(mShape->shapeImpl)
	);

	// TODO: Assume sphere...
	rbInfo.m_localInertia = btVector3(mMass, mMass, mMass);

	mRigidBody = new btRigidBody(rbInfo);
}

void RigidBodyComponent::Impl::update(Entity* e)
{
	if(mShape->isStatic())
		return;

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

RigidBodyComponent::RigidBodyComponent(DynamicsWorld& dynamicsWorld, float mass, const CollisionShapePtr& shape)
{
	mImpl = new Impl(dynamicsWorld, mass, shape);
}

RigidBodyComponent::~RigidBodyComponent(void)
{
	// mImpl may be null after onRemove() is called.
	if(mImpl) onRemove();
	delete mImpl;
}

Component* RigidBodyComponent::clone() const
{
	if(!mImpl) return nullptr;
	RigidBodyComponent* cloned = new RigidBodyComponent(*mImpl->mDynamicsWorld, mImpl->mMass, mImpl->mShape);
	return cloned;
}

void RigidBodyComponent::update(float)
{
	MCD_ASSUME(mImpl);
	if(Entity* e = entity())
		mImpl->update(e);
}

// TODO: If physics component is going to use multi-thread,
// we should delay the following operations till update() to do.
void RigidBodyComponent::applyForce(const Vec3f& force, const Vec3f& rel_pos)
{
	MCD_ASSUME(mImpl && mImpl->mRigidBody);
	mImpl->mRigidBody->applyForce(toBullet(force), toBullet(rel_pos));
}

float RigidBodyComponent::getLinearDamping() const
{
	MCD_ASSUME(mImpl && mImpl->mRigidBody);
	return mImpl->mRigidBody->getLinearDamping();
}

float RigidBodyComponent::getAngularDamping() const
{
	MCD_ASSUME(mImpl && mImpl->mRigidBody);
	return mImpl->mRigidBody->getAngularDamping();
}

void RigidBodyComponent::setDamping(float lin_damping, float ang_damping)
{
	MCD_ASSUME(mImpl && mImpl->mRigidBody);
	mImpl->mRigidBody->setDamping(lin_damping, ang_damping);
}

void RigidBodyComponent::onAdd()
{
	MCD_ASSUME(mImpl && mImpl->mDynamicsWorld);
	if(Entity* e = entity()) {
		mImpl->onAdd(e);
		mImpl->mDynamicsWorld->addRigidBody(*this);
	}
}

void RigidBodyComponent::onRemove()
{
	MCD_ASSUME(mImpl);
	mImpl->mDynamicsWorld->removeRigidBody(*this);
	// NOTE: mImpl may now become null, because of threaded dynamics world's removeRigidBody()
}

void createStaticRigidBody(DynamicsWorld& dynamicsWorld, Entity& entityTree)
{
	for(ComponentPreorderIterator itr(&entityTree); !itr.ended(); itr.next()) {
		MeshComponent* meshComponent = dynamic_cast<MeshComponent*>(itr.current());
		if(!meshComponent || !meshComponent->mesh)
			continue;

		Entity* e = meshComponent->entity();
		MCD_ASSUME(e);

		// Create the phyiscs component
		RigidBodyComponent* rbc = new RigidBodyComponent(dynamicsWorld, 0, new StaticTriMeshShape(meshComponent->mesh));
		e->addComponent(rbc);
	}
}

}	// namespace MCD
