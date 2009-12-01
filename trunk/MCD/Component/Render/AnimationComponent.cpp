#include "Pch.h"
#include "AnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/AnimationInstance.h"
#include "../../Core/Math/Quaternion.h"
#include "../../Core/System/Thread.h"
#include <set>

static const bool cUseAnimationThread = true;

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

		// Update the Entity's transform from the interpolatedResult.
		const AnimationTrack::KeyFrames& frames = interpolatedResult;

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

AnimationComponent::AnimationComponent(AnimationThread& animThread)
	: animationInstance(*new MyAnimationInstance)
	, mAnimationInstanceHolder(static_cast<MyAnimationInstance*>(&animationInstance))
	, mAnimationThread(animThread)
{
	animThread.addAnimationComponent(*this);
}

AnimationComponent::~AnimationComponent()
{
	mAnimationThread.removeAnimationComponent(*this);
}

void AnimationComponent::update(float dt)
{
	Entity* e = entity();
	if(!e || !e->enabled)
		return;

	animationInstance.time += dt;

	if(!cUseAnimationThread)
		static_cast<MyAnimationInstance&>(animationInstance).update();

	// Update the Entity's transform from the interpolatedResult.
	// NOTE: This assignment will cause data race since the animation thread is keep updating
	// animationInstance).transform, but it isn't a problem at all because the matrix
	// change relativly smooth over time.
	e->localTransform = static_cast<MyAnimationInstance&>(animationInstance).transform;
}

class AnimationThread::Impl : public Thread::IRunnable, public Thread
{
public:
	~Impl()
	{
		// Ensure the thread is stopped before mAnimationInstances get destroyed.
		if(Thread::keepRun())
			Thread::wait();
	}

	sal_override void run(Thread& thread) throw()
	{
		mPaused = false;

		// A local container of animation instance shared pointer to minize mutex lock time
		typedef std::vector<AnimationInstancePtr> Anims;
		Anims anims;

		// TODO: Clamp the rate of animation update to the framerate.
		while(Thread::keepRun())
		{
			if(mPaused) {
				mSleep(10);
				continue;
			}

			{	ScopeLock lock(mMutex);
				anims.assign(mAnimationInstances.begin(), mAnimationInstances.end());
			}

			for(Anims::const_iterator i=anims.begin(); i != anims.end(); ++i)
				(*i)->update();

			mSleep(1);
		}
	}

	Mutex mMutex;
	bool mPaused;

	typedef AnimationComponent::AnimationInstancePtr AnimationInstancePtr;
	std::set<AnimationInstancePtr> mAnimationInstances;
};	// Impl

AnimationThread::AnimationThread()
	: mImpl(*new Impl)
{
	if(cUseAnimationThread)
		mImpl.start(mImpl, false);
}

AnimationThread::~AnimationThread()
{
	delete &mImpl;
}

void AnimationThread::pause(bool p)
{
	mImpl.mPaused = p;
}

void AnimationThread::addAnimationComponent(AnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mAnimationInstances.insert(ac.mAnimationInstanceHolder);
}

void AnimationThread::removeAnimationComponent(AnimationComponent& ac)
{
	ScopeLock lock(mImpl.mMutex);
	mImpl.mAnimationInstances.erase(ac.mAnimationInstanceHolder);
}

}	// namespace MCD
