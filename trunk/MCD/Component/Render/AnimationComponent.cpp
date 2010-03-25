#include "Pch.h"
#include "AnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/AnimationInstance.h"
#include "../../Core/Math/Quaternion.h"
#include "../../Core/System/TaskPool.h"
#include <set>

namespace MCD {

struct Data
{
	void* data;
	AnimationComponent::Callback callback;
	AnimationComponentPtr animationComponent;
};	// Data

static void eventCallback(const AnimationInstance::Event& e)
{
	if(Data* data = reinterpret_cast<Data*>(e.data)) {
		if(data->callback && data->animationComponent)
			data->callback(*data->animationComponent, e.virtualFrameIdx, data->data);
	}
}

static void eventDestroy(void* eventData)
{
	delete reinterpret_cast<Data*>(eventData);
}

static void setEvent(AnimationComponent& c, AnimationInstance::Events& events, const char* weightedTrackName, size_t virtualFrameIdx, void* data)
{
	AnimationInstance::WeightedTrack* wt = c.animationInstance.getTrack(weightedTrackName);
	if(!wt)
		return;

	std::auto_ptr<Data> d(new Data);
	d->data = data;
	d->callback = c.callback;
	d->animationComponent = &c;

	if(AnimationInstance::Event* e = wt->edgeEvents.setEvent(virtualFrameIdx, d.get())) {
		e->data = d.release();
		e->callback = &eventCallback;
		e->destroyData = &eventDestroy;
	}
}

void AnimationComponent::setEdgeEvent(const char* weightedTrackName, size_t virtualFrameIdx, void* data)
{
	if(AnimationInstance::WeightedTrack* wt = animationInstance.getTrack(weightedTrackName))
		setEvent(*this, wt->edgeEvents, weightedTrackName, virtualFrameIdx, data);
}

void AnimationComponent::setLevelEvent(const char* weightedTrackName, size_t virtualFrameIdx, void* data)
{
	if(AnimationInstance::WeightedTrack* wt = animationInstance.getTrack(weightedTrackName))
		setEvent(*this, wt->levelEvents, weightedTrackName, virtualFrameIdx, data);
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
	: callback(nullptr)
	, destroyData(nullptr)
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

	void update()
	{
		if(mTaskPool) {
			// NOTE: Only schedule the task if it's not running, such that time is not
			// wasted for update an animation multiple times (with a not updated time value).
			if(!mIsUpdating)
				(void)mTaskPool->enqueue(*this);
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
