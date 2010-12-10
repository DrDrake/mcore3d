#ifndef __MCD_FRAMEWORK_ARCBALLCOMPONENT__
#define __MCD_FRAMEWORK_ARCBALLCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

typedef IntrusiveWeakPtr<Entity> EntityPtr;
typedef IntrusiveWeakPtr<class InputComponent> InputComponentPtr;

/*!	A First-Person-Shoot like controller component to move around a target Entity.
 */
class MCD_FRAMEWORK_API ArcBallComponent : public BehaviourComponent
{
public:
	ArcBallComponent();

	sal_override void update(float dt);

// Attributes
	EntityPtr target;	//!< The entity that we want to control
	InputComponentPtr inputComponent;

	float radius;
	float minRadius;
	float rotationSpeed;
	float translationSpeed;

protected:
	Vec2f mLastMouseAxis;
	Vec2f mAccumulateRotation;
	Vec2f mAccumulateTranslation;
};	// ArcBallComponent

typedef IntrusiveWeakPtr<ArcBallComponent> ArcBallComponentPtr;

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_ARCBALLCOMPONENT__
