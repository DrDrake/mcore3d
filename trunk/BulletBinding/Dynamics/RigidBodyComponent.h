#ifndef __MCD_BULLETBINDING_RIGIDBODYCOMPONENT__
#define __MCD_BULLETBINDING_RIGIDBODYCOMPONENT__

#include "../ShareLib.h"
#include "../../MCD/Core/Math/Vec3.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include <memory>

class btCollisionShape;
class btMotionState;
class btRigidBody;

namespace MCD {

namespace BulletBinding {

class MCD_BULLETBINDING_API RigidBodyComponent : public BehaviourComponent
{
public:
	RigidBodyComponent(float mass, btCollisionShape* shape);
	virtual ~RigidBodyComponent(void);

	// Override from BehaviourComponent
	sal_override void update();

	void onAttach();

	btRigidBody* getRightBody() const {
		return mRigidBody.get();
	}

	void activate();
	void applyForce(const Vec3f& force, const Vec3f& rel_pos);

	float getLinearDamping() const;
	float getAngularDamping() const;

	void setDamping(float lin_damping, float ang_damping);
	float mMass;

private:
	std::auto_ptr<btRigidBody> mRigidBody;
	std::auto_ptr<btMotionState> mMotionState;
	std::auto_ptr<btCollisionShape> mShape;
};	// RigidBodyComponent

}	// BulletBinding

}	// MCD

#endif	// __MCD_BULLETBINDING_RIGIDBODYCOMPONENT__
