#include "Pch.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/Math/AnimationState.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/System/Utility.h"

namespace MCD {

static AnimationUpdaterComponent* gAnimationUpdater = nullptr;

AnimationComponent::AnimationComponent()
	: pose(nullptr, 0)
{
}

AnimationComponent::~AnimationComponent()
{
	realloc(pose.getPtr(), 0);
}

Component* AnimationComponent::clone() const
{
	return nullptr;
}

void AnimationComponent::gather()
{
	MCD_ASSUME(gAnimationUpdater);
	gAnimationUpdater->mAnimationComponents.push_back(this);
}

void AnimatedComponent::gather()
{
	MCD_ASSUME(gAnimationUpdater);
	gAnimationUpdater->mAnimatedComponents.push_back(this);
}

void AnimationComponent::update(float worldTime)
{
	if(animations.empty()) return;

	const size_t trackCount = animations[0].clip->trackCount();

	if(pose.size < trackCount)
		initPose(trackCount);
	memset(pose.data, 0, pose.sizeInByte());

	float w = 0;

	MCD_FOREACH(const AnimationState& a_, animations)
	{
		AnimationState& a = const_cast<AnimationState&>(a_);
		a.worldTime = worldTime;
		if(a.weight == 0) continue;
		MCD_ASSERT(a.clip);
		MCD_ASSERT(a.clip->trackCount() == trackCount);

		w = a.blendResultTo(pose, w);
	}

	MCD_ASSERT(Mathf::isNearEqual(1, w) && "All weight should sum up to one");
}

void AnimationComponent::initPose(size_t trackCount)
{
	pose.data = (char*)realloc(pose.getPtr(), sizeof(AnimationClip::Sample) * trackCount);
	pose.size = trackCount;
	memset(pose.data, 0, pose.sizeInByte());
}

void AnimationUpdaterComponent::begin()
{
	mAnimationComponents.clear();
	mAnimatedComponents.clear();
	gAnimationUpdater = this;
	mWorldTime = float(mTimer.get().asSecond());
}

void AnimationUpdaterComponent::end(float dt)
{
	// Update the animation data first
	MCD_FOREACH(AnimationComponent* c, mAnimationComponents)
		c->update(mWorldTime);

	// Then update the compoents that depends on animation data
	MCD_FOREACH(AnimatedComponent* c, mAnimatedComponents)
		c->update();

	gAnimationUpdater = nullptr;
}

}	// namespace MCD
