#include "Pch.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/Math/AnimationState.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/System/Log.h"
#include "../Core/System/TaskPool.h"
#include "../Core/System/ThreadedCpuProfiler.h"
#include "../Render/Color.h"
#include <set>

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
#include <queue>
#ifdef MCD_VC
#	pragma warning(pop)
#endif

namespace MCD {

class EventQueue;

//!	The life time of this event data follows that of AnimationComponent
struct EventData
{
	void* data;
	size_t virtualFrameIdx;
	AnimationComponent::Callback callback;
	AnimationComponent::DestroyData destroyData;
	AnimationComponentPtr animationComponent;
	EventQueue* evenQueue;

	void doCallback()
	{
		MCD_ASSERT(callback && animationComponent);
		AnimationComponent& a = *animationComponent.getNotNull();
		callback(a, virtualFrameIdx, data);
	}
};	// EventData

// We embed the AnimationComponentPtr so that we can check that the EventData is destroyed or not.
class EventQueue : protected std::deque<std::pair<EventData*, AnimationComponentPtr> >
{
public:
	// Run in animation updating thread
	void push(EventData& e)
	{
		ScopeLock lock(mMutex);
		push_back(std::make_pair(&e, e.animationComponent));
	}

	// This function should be run in main thread
	sal_maybenull EventData* pop()
	{
		ScopeLock lock(mMutex);

		// Loop until there is a valid event data
		while(!empty()) {
			std::pair<EventData*, AnimationComponentPtr> pair = front();
			pop_back();
			if(pair.first && pair.second)
				return pair.first;
		}
		return nullptr;
	}

	Mutex mMutex;
};	// EventQueue

class AnimationComponent::MyAnimationInstance : public AnimationInstance
{
public:
	// NOTE: Setting dirty flag to true shuch that animation data get initialized,
	// which makes everything more safe.
	MyAnimationInstance() : dirty(true) {}

	void update()
	{
		if(!AnimationInstance::isAllTrackCommited())
			return;

		AnimationInstance::update();

		ScopeLock lock(mutex);
		const size_t animDataCount = animData.size();
		for(size_t i=0; i<animDataCount; ++i) {
			// Update the Entity's transform from the weightedResult.
			Mat44f& mat = animData[i].transform;
			const AnimationInstance::KeyFrames& frames = weightedResult;
			const size_t subtrackIdxOffset = i * subtrackPerEntity;

			// Position
			mat.setTranslation(reinterpret_cast<const Vec3f&>(frames[subtrackIdxOffset + 0]));	// The fourth component is ignored

			// Rotation
			Mat33f m;
			reinterpret_cast<const Quaternionf&>(frames[subtrackIdxOffset + 1]).toMatrix(m);
			MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(m.determinant(), 1, 1e-5f));
			mat.setMat33(m);

			// Since the scale is always be one after appling the unit quaternion,
			// therefore using scaleBy() is faster than using setScale().
			mat.scaleBy(reinterpret_cast<const Vec3f&>(frames[subtrackIdxOffset + 2]));	// The fourth component is ignored

			// Update the color.
			animData[i].color = reinterpret_cast<const ColorRGBAf&>(frames[subtrackIdxOffset + 3]);
		}
	}

	mutable Mutex mutex;	// Mutex for thread-safe resize of \em animData
	struct AnimData { 
		AnimData() : transform(Mat44f::cIdentity), color(1, 1) {}
		Mat44f transform; ColorRGBAf color;
	};
	typedef std::vector<AnimData> AnimDataList;
	AnimDataList animData;
	AnimationComponentPtr backRef;
	/// Indicate this MyAnimationInstance had been updated by AnimationUpdaterComponent in this frame.
	/// Set to true in AnimationComponent::update(), set to false in AnimationUpdaterComponent::update()
	bool dirty;
};	// MyAnimationInstance

AnimationComponent::AnimationComponent(AnimationUpdaterComponent& updater)
	: defaultCallback(nullptr)
	, defaultDestroyData(nullptr)
	, animationInstance(*new MyAnimationInstance)
	, animationUpdater(&updater)
	, mAnimationInstanceHolder(static_cast<MyAnimationInstance*>(&animationInstance))
{
	static_cast<MyAnimationInstance&>(animationInstance).backRef = this;
	updater.addAnimationComponent(*this);
}

AnimationComponent::~AnimationComponent()
{
	if(animationUpdater)
		animationUpdater->removeAnimationComponent(*this);
}

Component* AnimationComponent::clone() const
{
	if(!animationUpdater)
		return nullptr;
	AnimationComponent* cloned = new AnimationComponent(*animationUpdater);
	cloned->animationInstance = this->animationInstance;
	static_cast<MyAnimationInstance&>(cloned->animationInstance).backRef = cloned;
	return cloned;
}

bool AnimationComponent::postClone(const Entity& src, Entity& dest)
{
	// Find the Component in the src tree that corresponding to this
	AnimationComponent* srcComponent = dynamic_cast<AnimationComponent*>(
		ComponentPreorderIterator::componentByOffset(src, ComponentPreorderIterator::offsetFrom(dest, *this))
	);

	if(!srcComponent)
		return false;

	affectingEntities.clear();
	for(size_t i=0; i<srcComponent->affectingEntities.size(); ++i) {
		if(EntityPtr targetSrc = srcComponent->affectingEntities[i]) {
			// Find the Component in the src tree that corresponding to referenceToAnother
			EntityPtr targetDest = EntityPreorderIterator::entityByOffset(dest, EntityPreorderIterator::offsetFrom(src, *targetSrc));
			if(targetDest)
				affectingEntities.push_back(targetDest);
		}
		else
			affectingEntities.push_back(nullptr);
	}

	return true;
}

void AnimationComponent::update(float dt)
{
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
	MyAnimationInstance& myAnim = static_cast<MyAnimationInstance&>(*mAnimationInstanceHolder);
	myAnim.dirty = true;

	const size_t animDataCount = myAnim.animData.size();
	const size_t affectingEntityCount = affectingEntities.size();

	if(affectingEntityCount > 0) {
		for(size_t i=0; i<affectingEntityCount; ++i) {
			if(const EntityPtr& e_ = affectingEntities[i]) {
				if(i < animDataCount) {	// Excessing registered affecting entity than the sub-track count will be ignored.
					e_->localTransform = myAnim.animData[i].transform;
					// TODO: Apply the color to RenderableComponent
				}
			}
		}
	}
	// If there is no affecting entity registered, affect on it's host entity.
	else if(animDataCount > 0) {
		e->localTransform = myAnim.animData[0].transform;
		// TODO: Apply the color to RenderableComponent
	}
}

float AnimationComponent::time() const {
	return animationInstance.time;
}

void AnimationComponent::setTime(float t)
{
	animationInstance.time = t;
	MyAnimationInstance& myAnim = static_cast<MyAnimationInstance&>(*mAnimationInstanceHolder);
	myAnim.update();
}

class AnimationUpdaterComponent::Impl : public MCD::TaskPool::Task
{
public:
	Impl(Entity* systemEntities) : Task(0), mTaskPool(nullptr), mPaused(false), mIsUpdating(false)
	{
		if(systemEntities) {
			if(TaskPoolComponent* c = systemEntities->findComponentInChildrenExactType<TaskPoolComponent>())
				mTaskPool = &c->taskPool;
		}
	}

	sal_override void run(Thread& thread)
	{
		realUpdate();
	}

	void update()
	{
		if(mTaskPool) {
			// NOTE: Only schedule the task if it's not running, such that time is not
			// wasted for update an animation multiple times (with a not updated time value).
			if(!mIsUpdating)
				(void)mTaskPool->enqueue(*this);

			// Consume the event callbacks in mEventQueue
			if(EventData* e = mEventQueue.pop())
				e->doCallback();
		}
		else
			realUpdate();
	}

	void realUpdate()
	{
		if(mPaused)
			return;

		ThreadedCpuProfiler::Scope cpuProfiler("AnimationUpdaterComponent::update");

		mIsUpdating = true;

		{	ScopeLock lock(mMutex);
			MCD_FOREACH(const AnimationInstancePtr& a, mAnimationInstances) {
				if(!a->dirty)
					continue;
				mTmpAnims.push_back(a);
				a->dirty = false;
			}
		}

		for(Anims::const_iterator i=mTmpAnims.begin(); i != mTmpAnims.end(); ++i) {
			AnimationComponent::MyAnimationInstance& a = **i;
			// NOTE: If we ignore Entity::enabled, a simple "a.update()" can make the job done.
//			ScopeLock lock(a.backRef.destructionMutex());
			if(AnimationComponent* c = a.backRef.get()) {
				// Poll for the sub-track count and adjust the MyAnimationInstance accordingly
				if(a.subtrackCount() != a.animData.size() * AnimationComponent::subtrackPerEntity) {
					if(c->animationInstance.subtrackCount() % AnimationComponent::subtrackPerEntity != 0)
						Log::format(Log::Warn, "AnimationComponent use at least %d sub-track per entity, but only %d sub-track are given to %d entities",
							AnimationComponent::subtrackPerEntity, c->animationInstance.subtrackCount(), c->affectingEntities.size()
						);
					ScopeLock lock2(a.mutex);
					a.animData.resize(c->animationInstance.subtrackCount() / AnimationComponent::subtrackPerEntity);
				}

				if(Entity* e = c->entity()) {
					if(e->enabled) {
//						lock.unlockAndCancel();
						a.update();
					}
				}
			}
		}

		mIsUpdating = false;
		mTmpAnims.clear();
	}

	sal_maybenull TaskPool* mTaskPool;

	Mutex mMutex;
	bool mPaused;
	volatile bool mIsUpdating;

	/// Store all registered animations
	typedef AnimationComponent::AnimationInstancePtr AnimationInstancePtr;
	std::set<AnimationInstancePtr> mAnimationInstances;

	/// A local container of animation instance shared pointer to minize mutex lock time
	typedef std::vector<AnimationInstancePtr> Anims;
	Anims mTmpAnims;

	EventQueue mEventQueue;
};	// Impl

AnimationUpdaterComponent::AnimationUpdaterComponent(Entity* systemEntities)
	: mImpl(*new Impl(systemEntities))
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

TaskPool* AnimationUpdaterComponent::taskPool() {
	return mImpl.mTaskPool;
}

}	// namespace MCD
