#include "Pch.h"
#include "AnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/AnimationInstance.h"
#include "../../Core/Math/Quaternion.h"
#include "../../Core/System/TaskPool.h"
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

TaskPool* AnimationUpdaterComponent::taskPool() {
	return mImpl.mTaskPool;
}

}	// namespace MCD
