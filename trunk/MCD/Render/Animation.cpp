#include "Pch.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/Math/AnimationInstance.h"
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

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "jkbindd")
#	else
#		pragma comment(lib, "jkbind")
#	endif
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

static void eventCallback(const AnimationInstance::Event& e)
{
	if(EventData* data = reinterpret_cast<EventData*>(e.data)) {
		if(data->callback && data->animationComponent) {
			if(data->evenQueue)
				data->evenQueue->push(*data);
			else {
				MCD_ASSERT(data->virtualFrameIdx == e.virtualFrameIdx);
				data->doCallback();
			}
		}
	}
}

static void eventDestroy(void* eventData)
{
	delete reinterpret_cast<EventData*>(eventData);
}

static void setEvent(
	AnimationComponent& c, AnimationInstance::Events& events, const char* weightedTrackName, size_t virtualFrameIdx, void* data,
	AnimationComponent::Callback callback, AnimationComponent::DestroyData destroyData, EventQueue* eventQueue)
{
	callback = callback == nullptr ? c.defaultCallback : callback;
	destroyData = destroyData == nullptr ? c.defaultDestroyData : destroyData;

	AnimationInstance::WeightedTrack* wt = c.animationInstance.getTrack(weightedTrackName);
	if(!wt) {
		if(destroyData)
			destroyData(data);
		return;
	}

	std::auto_ptr<EventData> d(new EventData);
	d->data = data;
	d->virtualFrameIdx = virtualFrameIdx;
	d->callback = callback;
	d->destroyData = destroyData;
	d->animationComponent = &c;
	d->evenQueue = eventQueue;

	if(AnimationInstance::Event* e = events.setEvent(virtualFrameIdx, d.get())) {
		e->data = d.release();
		e->callback = &eventCallback;
		e->destroyData = &eventDestroy;
	}
}

class AnimationComponent::MyAnimationInstance : public AnimationInstance
{
public:
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
	// TODO: Handle disabled AnimationComponent in AnimationUpdaterComponent, or
	// only submit enabled animation components to the updater in this function.
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
			mAnims.resize(mAnimationInstances.size());
			mAnims.assign(mAnimationInstances.begin(), mAnimationInstances.end());
		}

		for(Anims::const_iterator i=mAnims.begin(); i != mAnims.end(); ++i) {
			AnimationComponent::MyAnimationInstance& a = **i;
			// NOTE: If we ignore Entity::enabled, a simple "a.update()" can make the job done.
			ScopeLock lock(a.backRef.destructionMutex());
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
						lock.mutex().unlock();
						lock.cancel();
						a.update();
					}
				}
			}
		}

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

	EventQueue mEventQueue;
};	// Impl

void AnimationComponent::setEdgeEvent(const char* weightedTrackName, size_t virtualFrameIdx, void* data, Callback callback, DestroyData destroyData)
{
	if(AnimationInstance::WeightedTrack* wt = animationInstance.getTrack(weightedTrackName)) {
		EventQueue* q = animationUpdater->taskPool() ? &animationUpdater->mImpl.mEventQueue : nullptr;
		setEvent(*this, wt->edgeEvents, weightedTrackName, virtualFrameIdx, data, callback, destroyData, q);
	}
}

void AnimationComponent::setLevelEvent(const char* weightedTrackName, size_t virtualFrameIdx, void* data, Callback callback, DestroyData destroyData)
{
	if(AnimationInstance::WeightedTrack* wt = animationInstance.getTrack(weightedTrackName)) {
		EventQueue* q = animationUpdater->taskPool() ? &animationUpdater->mImpl.mEventQueue : nullptr;
		setEvent(*this, wt->levelEvents, weightedTrackName, virtualFrameIdx, data, callback, destroyData, q);
	}
}

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
