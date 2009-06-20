#ifndef __MCD_COMPONENT_RIGIDBODYCOMPONENT__
#define __MCD_COMPONENT_RIGIDBODYCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

class CollisionShape;

/*!	Represent a rigid body, the most frequent component that you may need
	to work with when using physics components.

	It
 */
class MCD_COMPONENT_API RigidBodyComponent : public BehaviourComponent
{
	friend class DynamicsWorld;

public:
	//! It will take ownership of shape.
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

}	// MCD

#endif	// __MCD_COMPONENT_RIGIDBODYCOMPONENT__
