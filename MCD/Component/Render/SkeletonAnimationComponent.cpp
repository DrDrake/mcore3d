#include "Pch.h"
#include "SkeletonAnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/Skeleton.h"
#include "../../Core/System/TaskPool.h"
#include "../../Core/System/ThreadedCpuProfiler.h"
#include <map>

namespace MCD {

SkeletonAnimationComponent::SkeletonAnimationComponent(SkeletonAnimationUpdaterComponent& updater)
	: mSkeletonAnimation(new SkeletonAnimation(""))
	, skeletonAnimation(*mSkeletonAnimation)
	, animationUpdater(&updater)
{
	updater.addSkeletonAnimationComponent(*this);
}

SkeletonAnimationComponent::~SkeletonAnimationComponent()
{
	if(animationUpdater)
		animationUpdater->removeSkeletonAnimationComponent(*this);
}

bool SkeletonAnimationComponent::cloneable() const {
	return animationUpdater != nullptr;
}

Component* SkeletonAnimationComponent::clone() const
{
	if(!animationUpdater)
		return nullptr;
	SkeletonAnimationComponent* cloned = new SkeletonAnimationComponent(*animationUpdater);
	cloned->pose = this->pose;
	cloned->skeletonAnimation.anim = this->skeletonAnimation.anim;
	cloned->skeletonAnimation.skeleton = this->skeletonAnimation.skeleton;
	return cloned;
}

void SkeletonAnimationComponent::update(float dt)
{
	// TODO: Handle disabled SkeletonAnimationComponent in SkeletonAnimationUpdaterComponent
	Entity* e = entity();
	if(!e || !e->enabled)
		return;

	skeletonAnimation.anim.time += dt;

	// NOTE: The actual update() is NOT performed right here, MANY updates will
	// be batched together and processed in SkeletonAnimationUpdaterComponent later in time.
}

// TODO: Combine this with AnimationUpdaterComponent to reduce duplicated code
class SkeletonAnimationUpdaterComponent::Impl : public MCD::TaskPool::Task
{
public:
	Impl(TaskPool* taskPool) : Task(0), mTaskPool(taskPool), mPaused(false), mIsUpdating(false) {}

	sal_override void run(Thread& thread) throw()
	{
		realUpdate();
	}

	void update()
	{
		if(mTaskPool) {
			// NOTE: Only schedule the task if it's not running, such that time is not
			// wasted for update an animation multiple times (with a not updated time value).
			if(!mIsUpdating)
				(void)mTaskPool->enqueue(*this);	//NOTE: Simply ignore request if it's already in the queue
		}
		else
			realUpdate();
	}

	void realUpdate()
	{
		if(mPaused)
			return;

		ThreadedCpuProfiler::Scope cpuProfiler("SkeletonAnimationUpdaterComponent::update");

		mIsUpdating = true;

		{	ScopeLock lock(mMutex);
			mAnims.resize(mSkeletonAnimations.size());
			mAnims.assign(mSkeletonAnimations.begin(), mSkeletonAnimations.end());
		}

		for(Anims::const_iterator i=mAnims.begin(); i != mAnims.end(); ++i) {
			i->first->anim.update();

			// Ensure the SkeletonAnimationComponent will not be deleted within this scope
			ScopeLock lock(i->second.destructionMutex());

			if(!i->second)
				continue;

			i->second->pose.rootJointTransform() = Mat44f::cIdentity;
			i->first->applyTo(i->second->pose);
		}

		mIsUpdating = false;
	}

	sal_maybenull TaskPool* mTaskPool;

	Mutex mMutex;
	bool mPaused;
	volatile bool mIsUpdating;

	// The storage of SkeletonAnimationPtr keeps the resource valid while the thread is updating.
	std::map<SkeletonAnimationPtr, SkeletonAnimationComponentPtr> mSkeletonAnimations;

	// A local container of animation instance shared pointer to minize mutex lock time
	typedef std::vector<std::pair<SkeletonAnimationPtr, SkeletonAnimationComponentPtr> > Anims;
	Anims mAnims;
};	// Impl

SkeletonAnimationUpdaterComponent::SkeletonAnimationUpdaterComponent(TaskPool* taskPool)
	: mImpl(*new Impl(taskPool))
{
}

SkeletonAnimationUpdaterComponent::~SkeletonAnimationUpdaterComponent()
{
	delete &mImpl;
}

void SkeletonAnimationUpdaterComponent::update(float dt)
{
	mImpl.update();
}

void SkeletonAnimationUpdaterComponent::pause(bool p)
{
	mImpl.mPaused = p;
}

void SkeletonAnimationUpdaterComponent::addSkeletonAnimationComponent(SkeletonAnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mSkeletonAnimations[&ac.skeletonAnimation] = &ac;
}

void SkeletonAnimationUpdaterComponent::removeSkeletonAnimationComponent(SkeletonAnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mSkeletonAnimations.erase(&ac.skeletonAnimation);
}

}	// namespace MCD
