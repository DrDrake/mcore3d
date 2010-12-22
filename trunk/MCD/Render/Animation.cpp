#include "Pch.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/Math/AnimationState.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Utility.h"

namespace MCD {

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
	return float(Timer::sinceProgramStatup().asSecond());
}

void AnimationUpdaterComponent::begin()
{
	mAnimationComponents.clear();
	mAnimatedComponents.clear();
	gAnimationUpdater = this;
	mWorldTime = worldTime();
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
	blendTree.worldTime = worldTime;
	pose = blendTree.getFinalPose();
}

}	// namespace MCD
