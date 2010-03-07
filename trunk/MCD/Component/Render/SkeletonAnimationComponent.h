#ifndef __MCD_COMPONENTS_RENDER_SKELETONANIMATIONCOMPONENT__
#define __MCD_COMPONENTS_RENDER_SKELETONANIMATIONCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/BehaviourComponent.h"
#include "../../Core/System/SharedPtr.h"

namespace MCD {

class TaskPool;
typedef IntrusivePtr<class SkeletonAnimation> SkeletonAnimationPtr;
typedef IntrusiveWeakPtr<class SkeletonAnimationUpdaterComponent> SkeletonAnimationUpdaterComponentPtr;
typedef IntrusiveWeakPtr<class SkinMeshComponent> SkinMeshComponentPtr;

//!	A component that use the AnimationInstance to control some aspects of an Entity.
class MCD_COMPONENT_API SkeletonAnimationComponent : public BehaviourComponent
{
protected:
	friend class SkeletonAnimationUpdaterComponent;

	/*!	Every SkeletonAnimationComponent has it's own instance of skeletonAnimation.
		\note This variable's declaration should appear before the reference \em skeletonAnimation.
	 */
	SkeletonAnimationPtr mSkeletonAnimation;

public:
	explicit SkeletonAnimationComponent(SkeletonAnimationUpdaterComponent& updater);

	sal_override ~SkeletonAnimationComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const;

	//!	Clone will fail if the associated SkeletonAnimationUpdaterComponent is already destroyed.
	sal_override sal_maybenull Component* clone() const;

// Operations
	sal_override void update(float dt);

// Attributes
	SkinMeshComponentPtr skinMesh;	//!< The animation will apply to skinMesh's pose

	SkeletonAnimation& skeletonAnimation;

	const SkeletonAnimationUpdaterComponentPtr animationUpdater;
};	// SkeletonAnimationComponent

typedef IntrusiveWeakPtr<SkeletonAnimationComponent> SkeletonAnimationComponentPtr;

/*!	Centralize the update of many SkeletonAnimationComponent, resulting better cache coherent.
	It can also be able to utilize TaskPool for animation update.
 */
class MCD_COMPONENT_API SkeletonAnimationUpdaterComponent : public BehaviourComponent
{
public:
	/*!	Multi-thread is used if \em taskPool is not null.
		User has to take care the life time of \em taskPool to be longer than this.
	 */
	explicit SkeletonAnimationUpdaterComponent(sal_in_opt TaskPool* taskPool);

	sal_override ~SkeletonAnimationUpdaterComponent();

// Operations
	sal_override void update(float dt);

	void pause(bool p);

	void addSkeletonAnimationComponent(SkeletonAnimationComponent& ac);

	void removeSkeletonAnimationComponent(SkeletonAnimationComponent& ac);

protected:
	class Impl;
	Impl& mImpl;
};	// SkeletonAnimationUpdaterComponent

}	// namespace MCD

#endif	// __MCD_COMPONENTS_RENDER_SKELETONANIMATIONCOMPONENT__
