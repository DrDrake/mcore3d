#pragma once

#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include <memory>

namespace MCD
{
	namespace BulletBinding
	{
		class RigidBodyComponent : public BehaviourComponent
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

			inline btRigidBody* getRightBody()
			{
				return mRigidBody.get();
			}
		};
	}
}
