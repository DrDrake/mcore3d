#ifndef __MCD_COMPONENT_RIGIDBODYCOMPONENT__
#define __MCD_COMPONENT_RIGIDBODYCOMPONENT__

#include "../ShareLib.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

namespace PhysicsComponent {

class CollisionShape;

class MCD_COMPONENT_API RigidBodyComponent : public BehaviourComponent
{
	friend class DynamicsWorld;

public:
	RigidBodyComponent(float mass, CollisionShape* shape);

	sal_override ~RigidBodyComponent(void);

	// Override from BehaviourComponent
	sal_override void update();

	void onAttach();

	void activate();

	void applyForce(const Vec3f& force, const Vec3f& rel_pos);

	float getLinearDamping() const;
	float getAngularDamping() const;

	void setDamping(float lin_damping, float ang_damping);

private:
	class Impl;
	Impl* mImpl;
};	// RigidBodyComponent

}	// PhysicsComponent

}	// MCD

#endif	// __MCD_COMPONENT_RIGIDBODYCOMPONENT__
