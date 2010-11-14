#include "Pch.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/Math/AnimationState.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Utility.h"

namespace MCD {

static Timer gTimer;	// Maintain the global time line

static AnimationUpdaterComponent* gAnimationUpdater = nullptr;

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

float AnimationUpdaterComponent::worldTime()
{
	return float(gTimer.get().asSecond());
}

void AnimationUpdaterComponent::begin()
{
	mAnimationComponents.clear();
	mAnimatedComponents.clear();
	gAnimationUpdater = this;
	mWorldTime = float(gTimer.get().asSecond());
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

SimpleAnimationComponent::SimpleAnimationComponent()
	: pose(nullptr, 0)
{
}

SimpleAnimationComponent::~SimpleAnimationComponent()
{
	free(pose.getPtr());
}

Component* SimpleAnimationComponent::clone() const
{
	return nullptr;
}

AnimationComponent::Pose& SimpleAnimationComponent::getPose()
{
	return pose;
}

void SimpleAnimationComponent::update(float worldTime)
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

void SimpleAnimationComponent::initPose(size_t trackCount)
{
	pose.data = (char*)realloc(pose.getPtr(), sizeof(AnimationClip::Sample) * trackCount);
	pose.size = trackCount;
	memset(pose.data, 0, pose.sizeInByte());
}

}	// namespace MCD
