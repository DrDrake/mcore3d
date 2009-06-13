#ifndef __MCD_BULLETBINDING_RIGIDBODYCOMPONENT__
#define __MCD_BULLETBINDING_RIGIDBODYCOMPONENT__

#include "../ShareLib.h"

#pragma warning( push )
#pragma warning( disable : 4100 )
#include "../../3Party/bullet/btBulletDynamicsCommon.h"
#include "../../3Party/bullet/btBulletCollisionCommon.h"
#pragma warning( pop )

#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include <memory>

namespace MCD
{
namespace BulletBinding
{
	class MCD_BULLETBINDING_API RigidBodyComponent : public BehaviourComponent
	{
	private:
		std::auto_ptr<btRigidBody> mRigidBody;
		std::auto_ptr<btMotionState> mMotionState;
		std::auto_ptr<btCollisionShape> mShape;
	public:
		RigidBodyComponent(float mass, btCollisionShape* shape);
		virtual ~RigidBodyComponent(void);

		// Override from BehaviourComponent
		sal_override void update();

		void onAttach();

		inline btRigidBody* getRightBody() const
		{
			return mRigidBody.get();
		}

		void activate();
		void applyForce(const Vec3f& force, const Vec3f& rel_pos);

		float getLinearDamping() const;
		float getAngularDamping() const;

		void setDamping(float lin_damping, float ang_damping);
		float mMass;
	};
}
}

#endif