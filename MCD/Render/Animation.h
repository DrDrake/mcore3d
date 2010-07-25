#ifndef __MCD_RENDER_ANIMATION__
#define __MCD_RENDER_ANIMATION__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"
#include "../Core/System/SharedPtr.h"
#include "../../3Party/jkbind/Declarator.h"
#include <vector>

namespace MCD {

class AnimationInstance;
class TaskPool;
typedef IntrusiveWeakPtr<class Entity> EntityPtr;
typedef IntrusiveWeakPtr<class AnimationUpdaterComponent> AnimationUpdaterComponentPtr;

/*!	A component that use the AnimationInstance to control some aspects of an Entity.
	If the member variable \em affectingEntities is empty then this component will
	control over it's hosting Entity; otherwise all the sub-tracks will corresponds
	to the Entity in \em affectingEntities.
 */
class MCD_RENDER_API AnimationComponent : public BehaviourComponent
{
	class MyAnimationInstance;

public:
	explicit AnimationComponent(AnimationUpdaterComponent& updater);

	sal_override ~AnimationComponent();

// Cloning
	/*!	Clone will fail if the associated AnimationUpdaterComponent is already destroyed.
		The clone function will not clone the animation event (yet?).
	 */
	sal_override sal_maybenull Component* clone() const;
	sal_override sal_checkreturn bool postClone(const Entity& src, Entity& dest);

// Operations
	sal_override void update(float dt);

// Event
	typedef void (*Callback)(AnimationComponent& c, size_t virtualFrameIdx, void* eventData);
	Callback defaultCallback;

	typedef void (*DestroyData)(void* eventData);
	DestroyData defaultDestroyData;

	//!	The parameter \em data will be cleanup by \em AnimationComponent::destroyData immediatly if the operation failed.
	void setEdgeEvent(
		sal_in_z const char* weightedTrackName, size_t virtualFrameIdx, sal_maybenull void* data,
		sal_maybenull Callback callback=nullptr, sal_maybenull DestroyData destroyData=nullptr
	);

	void setLevelEvent(
		sal_in_z const char* weightedTrackName, size_t virtualFrameIdx, sal_maybenull void* data,
		sal_maybenull Callback callback=nullptr, sal_maybenull DestroyData destroyData=nullptr
	);

	script::Event<
		void, script::plain, size_t,
		script::objNoCare, AnimationComponent*,
		script::plain, const char*
	> scriptCallback;

// Attributes
	/*!	Sub-track,	usage
		0		->	Position	(Linear)
		1		->	Orientation	(Slerp)
		2		->	Scale		(Linear)
		3		->	Color		(Linear)

		The number of sub-tracks in the AnimationInstance should be multiple of 4.
	 */
	AnimationInstance& animationInstance;

	static const size_t subtrackPerEntity = 4;

	std::vector<EntityPtr> affectingEntities;

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
class MCD_RENDER_API AnimationUpdaterComponent : public BehaviourComponent
{
public:
	/*!	Multi-thread is used if \em systemEntities is not null.
		User has to take care the life time of \em TaskPoolComponent to be longer than this.
	 */
	explicit AnimationUpdaterComponent(sal_maybenull Entity* systemEntities);

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

#endif	// __MCD_RENDER_ANIMATION__
