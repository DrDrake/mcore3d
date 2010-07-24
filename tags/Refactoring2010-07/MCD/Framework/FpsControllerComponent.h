#ifndef __MCD_FRAMEWORK_FPSCONTROLLERCOMPONENT__
#define __MCD_FRAMEWORK_FPSCONTROLLERCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

typedef IntrusiveWeakPtr<Entity> EntityPtr;
typedef IntrusiveWeakPtr<class InputComponent> InputComponentPtr;

/*!	A First-Person-Shoot like controller component to move around a target Entity.
 */
class MCD_FRAMEWORK_API FpsControllerComponent : public BehaviourComponent
{
public:
	FpsControllerComponent();

	sal_override void update(float dt);

// Attributes
	EntityPtr target;	//!< The entity that we want to control
	InputComponentPtr inputComponent;

	float translationSpeed;
	float mouseSensitivity;

protected:
	Vec2f mLastMouseAxis;
	Vec2f mAccumulateMouseDelta;
};	// FpsControllerComponent

typedef IntrusiveWeakPtr<FpsControllerComponent> FpsControllerComponentPtr;

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_FPSCONTROLLERCOMPONENT__
