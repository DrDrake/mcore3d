#ifndef __MCD_RENDER_ANIMATION__
#define __MCD_RENDER_ANIMATION__

#include "ShareLib.h"
#include "../Core/Entity/Component.h"
#include "../Core/Math/AnimationState.h"
#include "../Core/System/Timer.h"
#include <vector>

namespace MCD {

class AnimationUpdaterComponent;

/// This Component store animation data and provide interface on
/// how to orchestra the various AnimationClip to it's final pose.
///
/// This Component does tell how the data should be interpreted, but
/// derived class of AnimationViewComponent should.
class MCD_RENDER_API AnimationComponent : public Component
{
public:
	AnimationComponent();

	sal_override ~AnimationComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(AnimationComponent);
	}

// Cloning
	sal_override sal_maybenull Component* clone() const;

// Attributes
	/// The animation pose after blending all the AnimationState together.
	AnimationState::Pose pose;

	std::vector<AnimationState> animations;

protected:
	friend class AnimationUpdaterComponent;
	sal_override void gather();
	void update(float worldTime);
	void initPose(size_t trackCount);
};	// AnimationComponent

typedef IntrusiveWeakPtr<AnimationComponent> AnimationComponentPtr;

/// Component that will use the calculated data in AnimationComponent.
/// Concret class example are: SkeletonComponent and AnimatedTransform.
class MCD_ABSTRACT_CLASS MCD_RENDER_API AnimatedComponent : public Component
{
public:
// Operations
	virtual void update() = 0;

// Attributes
	AnimationComponentPtr animation;

protected:
	sal_override void gather();
};	// AnimatedComponent

/// Centralize the update of many AnimationComponent, to make the update order
/// more deterministic. It also resulting better memory cache usage too.
class MCD_RENDER_API AnimationUpdaterComponent : public ComponentUpdater
{
public:
// Operations
	sal_override void begin();
	sal_override void end(float dt);

	float worldTime() const { return mWorldTime; }

protected:
	friend class AnimationComponent;
	friend class AnimatedComponent;
	Timer mTimer;	// Maintain the global time line
	float mWorldTime;
	std::vector<AnimationComponent*> mAnimationComponents;
	std::vector<AnimatedComponent*> mAnimatedComponents;
};	// AnimationUpdaterComponent

typedef IntrusiveWeakPtr<AnimationUpdaterComponent> AnimationUpdaterComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_ANIMATION__
