#ifndef __MCD_COMPONENTS_RENDER_ANIMATIONCOMPONENT__
#define __MCD_COMPONENTS_RENDER_ANIMATIONCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"
#include "../../Core/System/SharedPtr.h"

namespace MCD {

class AnimationInstance;
class TaskPool;
typedef IntrusiveWeakPtr<class AnimationUpdaterComponent> AnimationUpdaterComponentPtr;

//!	A component that use the AnimationInstance to control some aspects of an Entity.
class MCD_COMPONENT_API AnimationComponent : public BehaviourComponent
{
	class MyAnimationInstance;

public:
	explicit AnimationComponent(AnimationUpdaterComponent& updater);

	sal_override ~AnimationComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const;

	//!	Clone will fail if the associated AnimationUpdaterComponent is already destroyed.
	sal_override sal_maybenull Component* clone() const;

// Operations
	sal_override void update(float dt);

// Event
	//!	The parameter \em data will be cleanup by \em AnimationComponent::destroyData immediatly if the operation failed.
	void setEdgeEvent(sal_in_z const char* weightedTrackName, size_t virtualFrameIdx, sal_maybenull void* data);

	void setLevelEvent(sal_in_z const char* weightedTrackName, size_t virtualFrameIdx, sal_maybenull void* data);

	typedef void (*Callback)(AnimationComponent& c, size_t virtualFrameIdx, void* eventData);
	Callback callback;

	typedef void (*DestroyData)(void* eventData);
	DestroyData destroyData;

// Attributes
	/*!	Sub-track,	usage
		0		->	Position	(Linear)
		1		->	Orientation	(Slerp)
		2		->	Scale		(Linear)
		3		->	Color		(Linear)
	 */
	AnimationInstance& animationInstance;

	const AnimationUpdaterComponentPtr animationUpdater;

protected:
	friend class AnimationUpdaterComponent;

	/*!	In order to decouple the multi-thread life-time problem,
		we share the AnimationInstance with the updater thread.
	 */
	typedef SharedPtr<MyAnimationInstance> AnimationInstancePtr;
	const AnimationInstancePtr mAnimationInstanceHolder;
};	// AnimationComponent

typedef IntrusiveWeakPtr<AnimationComponent> AnimationComponentPtr;

/*!	Centralize the update of many AnimationComponent, resulting better cache coherent.
	It can also be able to utilize TaskPool for animation update.
 */
class MCD_COMPONENT_API AnimationUpdaterComponent : public BehaviourComponent
{
public:
	/*!	Multi-thread is used if \em taskPool is not null.
		User has to take care the life time of \em taskPool to be longer than this.
	 */
	explicit AnimationUpdaterComponent(sal_in_opt TaskPool* taskPool);

	sal_override ~AnimationUpdaterComponent();

// Operations
	sal_override void update(float dt);

	void pause(bool p);

	void addAnimationComponent(AnimationComponent& ac);

	void removeAnimationComponent(AnimationComponent& ac);

// Attributes
	sal_maybenull TaskPool* taskPool();

protected:
	friend class AnimationComponent;
	class Impl;
	Impl& mImpl;
};	// AnimationUpdaterComponent

typedef IntrusiveWeakPtr<AnimationUpdaterComponent> AnimationUpdaterComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENTS_RENDER_ANIMATIONCOMPONENT__
