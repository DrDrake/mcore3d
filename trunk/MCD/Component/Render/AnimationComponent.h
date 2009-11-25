#ifndef __MCD_RENDER_COMPONENTS_ANIMATIONCOMPONENT__
#define __MCD_RENDER_COMPONENTS_ANIMATIONCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"
#include "../../Core/System/SharedPtr.h"

#include "../../Core/System/Timer.h"

namespace MCD {

class AnimationInstance;

class MCD_COMPONENT_API AnimationComponent : public BehaviourComponent
{
public:
	AnimationComponent();

	sal_override ~AnimationComponent();

// Operations
	sal_override void update();

// Attrubutes
	/*!	Sub-track,	usage
		0		->	Position	(Linear)
		1		->	Orientation	(Slerp)
		2		->	Scale		(Linear)
		3		->	Color		(Linear)
	 */
	AnimationInstance& animationInstance;

	// TODO: Remove the use of timer here
	DeltaTimer timer;

protected:
	/*!	In order to decouple the multi-thread life-time problem,
		we share the AnimationInstance with the animation update thread.
	 */
	typedef SharedPtr<AnimationInstance> AnimationInstancePtr;	// TODO: Thread safe to use SharedPtr?
	const AnimationInstancePtr animationInstanceHolder;
};	// AnimationComponent

typedef WeakPtr<AnimationComponent> AnimationComponentPtr;

class MCD_COMPONENT_API AnimationThread
{
public:
// Operations
	void update();

	void addAnimationComponent(AnimationComponent& ac);

	void removeAnimationComponent(AnimationComponent& ac);
};	// AnimationThreadComponent

}	// namespace MCD

#endif	// __MCD_RENDER_COMPONENTS_ANIMATIONCOMPONENT__
