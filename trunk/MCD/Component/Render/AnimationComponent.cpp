#include "Pch.h"
#include "AnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/AnimationInstance.h"
#include "../../Core/Math/Quaternion.h"
#include "../../Core/System/TaskPool.h"
#include <set>

namespace MCD {

class AnimationComponent::MyAnimationInstance : public AnimationInstance
{
public:
	MyAnimationInstance()
	{
		transform = Mat44f::cIdentity;
	}

	void update()
	{
		if(!AnimationInstance::isAllTrackCommited())
			return;

		AnimationInstance::update();

		// Update the Entity's transform from the weightedResult.
		const AnimationInstance::KeyFrames& frames = weightedResult;

		// Position
		if(frames.size > 0)
			transform.setTranslation(reinterpret_cast<const Vec3f&>(frames[0]));	// The fourth component is ignored

		if(frames.size > 1) {
			Mat33f m;
			reinterpret_cast<const Quaternionf&>(frames[1]).toMatrix(m);
			MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(m.determinant(), 1, 1e-5f));
			transform.setMat33(m);
		}

		// Since the scale is always be one after appling the unit quaternion,
		// therefore using scaleBy() is faster than using setScale().
		if(frames.size > 2)
			transform.scaleBy(reinterpret_cast<const Vec3f&>(frames[2]));	// The fourth component is ignored

		// TODO: Update the color.
	}

	Mat44f transform;
};	// MyAnimationInstance

AnimationComponent::AnimationComponent(AnimationUpdaterComponent& updater)
	: animationInstance(*new MyAnimationInstance)
	, animationUpdater(&updater)
	, mAnimationInstanceHolder(static_cast<MyAnimationInstance*>(&animationInstance))
{
	updater.addAnimationComponent(*this);
}

AnimationComponent::~AnimationComponent()
{
	if(animationUpdater)
		animationUpdater->removeAnimationComponent(*this);
}

bool AnimationComponent::cloneable() const {
	return animationUpdater != nullptr;
}

Component* AnimationComponent::clone() const
{
	if(!animationUpdater)
		return nullptr;
	AnimationComponent* cloned = new AnimationComponent(*animationUpdater);
	cloned->animationInstance = this->animationInstance;
	return cloned;
}

void AnimationComponent::update(float dt)
{
	// TODO: Handle disabled AnimationComponent in AnimationUpdaterComponent
	Entity* e = entity();
	if(!e || !e->enabled)
		return;

	animationInstance.time += dt;

	// NOTE: The actual update() is NOT performed right here, MANY updates will
	// be batched together and processed in AnimationUpdaterComponent later in time.

	// Update the Entity's transform from the interpolatedResult.
	// NOTE: This assignment will cause data race since the animation thread is keep updating
	// animationInstance).transform, but it isn't a problem at all because the matrix
	// change relativly smooth over time.
	e->localTransform = static_cast<MyAnimationInstance&>(animationInstance).transform;
}

class AnimationUpdaterComponent::Impl : public MCD::TaskPool::Task
{
public:
	Impl(TaskPool* taskPool) : Task(0), mTaskPool(taskPool), mPaused(false), mIsUpdating(false) {}

	sal_override void run(Thread& thread) throw()
	{
		realUpdate();
	}

	MCD_NOINLINE void update()
	{
		if(mTaskPool) {
			// NOTE: Only schedule the task if it's not running, such that time is not
			// wasted for update an animation multiple times (with a not updated time value).
			if(!mIsUpdating)
				mTaskPool->enqueue(*this);
		}
		else
			realUpdate();
	}

	void realUpdate()
	{
		if(mPaused)
			return;

		mIsUpdating = true;

		{	ScopeLock lock(mMutex);
			mAnims.resize(mAnimationInstances.size());
			mAnims.assign(mAnimationInstances.begin(), mAnimationInstances.end());
		}

		for(Anims::const_iterator i=mAnims.begin(); i != mAnims.end(); ++i)
			(*i)->update();

		mIsUpdating = false;
	}

	sal_maybenull TaskPool* mTaskPool;

	Mutex mMutex;
	bool mPaused;
	volatile bool mIsUpdating;

	typedef AnimationComponent::AnimationInstancePtr AnimationInstancePtr;
	std::set<AnimationInstancePtr> mAnimationInstances;

	// A local container of animation instance shared pointer to minize mutex lock time
	typedef std::vector<AnimationInstancePtr> Anims;
	Anims mAnims;
};	// Impl

AnimationUpdaterComponent::AnimationUpdaterComponent(TaskPool* taskPool)
	: mImpl(*new Impl(taskPool))
{
}

AnimationUpdaterComponent::~AnimationUpdaterComponent()
{
	delete &mImpl;
}

void AnimationUpdaterComponent::update(float dt)
{
	mImpl.update();
}

void AnimationUpdaterComponent::pause(bool p)
{
	mImpl.mPaused = p;
}

void AnimationUpdaterComponent::addAnimationComponent(AnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mAnimationInstances.insert(ac.mAnimationInstanceHolder);
}

void AnimationUpdaterComponent::removeAnimationComponent(AnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mAnimationInstances.erase(ac.mAnimationInstanceHolder);
}

}	// namespace MCD
