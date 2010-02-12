#include "Pch.h"
#include "AnimationTrack.h"
#include "BasicFunction.h"
#include "Vec4.h"
#include "../System/MemoryProfiler.h"
#include <math.h>	// for fmodf()

namespace MCD {

AnimationTrack::ScopedReadLock::ScopedReadLock(const AnimationTrack& a) : track(a) {
	a.acquireReadLock();
}

AnimationTrack::ScopedReadLock::~ScopedReadLock() {
	track.releaseReadLock();
}

AnimationTrack::ScopedWriteLock::ScopedWriteLock(const AnimationTrack& a) : track(a) {
	a.acquireWriteLock();
}

AnimationTrack::ScopedWriteLock::~ScopedWriteLock() {
	track.releaseWriteLock();
}

AnimationTrack::Interpolation::Interpolation() {
	::memset(this, 0, sizeof(*this));
}

AnimationTrack::AnimationTrack(const Path& fileId)
	: Resource(fileId)
	, keyframes(nullptr, 0)
	, subtracks(nullptr, 0)
	, loop(true), naturalFramerate(1)
	, mTotalTime(0)
	, mCommitted(false)
{
}

AnimationTrack::~AnimationTrack()
{
	::free(keyframes.getPtr());
	::free(subtracks.getPtr());
}

bool AnimationTrack::init(const StrideArray<const size_t>& subtrackFrameCount)
{
	if(subtrackFrameCount.isEmpty())
		return false;

	MCD_ASSERT(mMutex.isLocked());

	if(isCommitted())
		return false;

	::free(keyframes.getPtr());
	::free(subtracks.getPtr());

	// Find out the total key frame count of all sub-tracks
	size_t totalFrameCount = 0;
	for(size_t i=0; i<subtrackFrameCount.size; ++i)
		totalFrameCount += subtrackFrameCount[i];

	const size_t subtrackCount = subtrackFrameCount.size;
	MemoryProfiler::Scope scope("AnimationTrack::init");
	keyframes = KeyFrames(::malloc(totalFrameCount * sizeof(KeyFrame)), totalFrameCount);
	subtracks = Subtracks(::malloc(subtrackCount * sizeof(Subtrack)), subtrackCount);

	::memset(keyframes.data, 0, keyframes.sizeInByte());
	::memset(subtracks.data, 0, subtracks.sizeInByte());

	size_t j = 0;
	for(size_t i=0; i<subtracks.size; ++i) {
		const_cast<size_t&>(subtracks[i].index) = j;
		j += (subtracks[i].frameCount = subtrackFrameCount[i]);
		subtracks[i].flag = Linear;
	}

	return true;
}

size_t AnimationTrack::subtrackCount() const
{
	// No need to check for the lock since only init() will modify it.
	MCD_ASSERT(true || mMutex.isLocked());
	return subtracks.size;
}

size_t AnimationTrack::keyframeCount(size_t index) const
{
	MCD_ASSERT(mMutex.isLocked());

	if(index < subtrackCount())
		return subtracks[index].frameCount;
	return 0;
}

float AnimationTrack::totalTime(size_t index) const
{
	MCD_ASSERT(mMutex.isLocked());

	if(index < subtrackCount()) {
		KeyFrames f = const_cast<AnimationTrack*>(this)->getKeyFramesForSubtrack(index);
		return f[subtracks[index].frameCount - 1].time;
	}
	return 0;
}

float AnimationTrack::totalTime() const
{
	// No need to check for the lock since only releaseWriteLock() will modify it.
	MCD_ASSERT(true || mMutex.isLocked());
	return mTotalTime;
}

AnimationTrack::KeyFrames AnimationTrack::getKeyFramesForSubtrack(size_t index)
{
	MCD_ASSERT(mMutex.isLocked());

	if(index < subtrackCount())
		return KeyFrames(&keyframes[subtracks[index].index], subtracks[index].frameCount);

	MCD_ASSERT(false && "AnimationTrack::getKeyFramesForSubtrack out of range");
	return KeyFrames(nullptr, 0);
}

void AnimationTrack::interpolate(float time, const Interpolations& result) const
{
	acquireReadLock();
	interpolateNoLock(time, result);
	releaseReadLock();
}

void AnimationTrack::interpolateNoLock(float time, const Interpolations& result) const
{
	for(size_t i=0; i<subtracks.size; ++i)
		interpolateSingleSubtrack(time, result[i], i);
}

void AnimationTrack::interpolateSingleSubtrack(float time, Interpolation& result, size_t trackIndex) const
{
	MCD_ASSERT(mMutex.isLocked());

	if(!mCommitted)
		return;

	KeyFrames frames = const_cast<AnimationTrack*>(this)->getKeyFramesForSubtrack(trackIndex);

	// If the animation has only one frame, there is no need to 
	// do any interpolation, simply copy the data.
	if(frames.size < 2) {
		::memcpy(result.v, frames[0].v, sizeof(result.v));
		return;
	}

	{	// Phase 1: find the wrapped version of time
		if(loop)
			time = ::fmodf(time, totalTime());
		else
			time = Mathf::clamp(time, time, totalTime(trackIndex));
	}

	{	// Phase 2: Find the current and pervious frame index
		size_t curr = frames[result.frame1Idx].time < time ? result.frame1Idx : 0;

		for(size_t i=curr; i < frames.size; ++i)
			if(frames[i].time >= time) { curr = i; break; }

		result.frame2Idx = (curr == 0) ? 1 : size_t(curr);
		result.frame1Idx = result.frame2Idx - 1;
	}

	{	// Phase 3: compute the weight between the frame1Idx and frame2Idx
		const float t1 = frames[result.frame1Idx].time;
		const float t2 = frames[result.frame2Idx].time;

		MCD_ASSUME(t2 > t1);
		result.ratio = (time - t1) / (t2 - t1);
	}

	// Phase 4: perform interpolation
	const Vec4f& f1 = reinterpret_cast<const Vec4f&>(frames[result.frame1Idx]);
	const Vec4f& f2 = reinterpret_cast<const Vec4f&>(frames[result.frame2Idx]);
	Vec4f& o = reinterpret_cast<Vec4f&>(result.v);

	const Flags flag = subtracks[trackIndex].flag;
	if(flag == Linear)
	{
	Linear:
		o = f1 + result.ratio * (f2 - f1);
	}
	else if(flag == Slerp)
	{
		// Refernece: From ID software, "Slerping Clock Cycles"
		const float cosVal = f1 % f2;
		const float absCosVal = fabs(cosVal);
		if((1.0f - absCosVal) > 1e-6f)
		{
			// Standard case (slerp)
			const float sinSqr = 1.0f - absCosVal * absCosVal;
			const float invSin = 1.0f / sqrt(sinSqr);
			const float omega = Mathf::aTanPositive(sinSqr * invSin, absCosVal);
			const float scale0 = Mathf::sinZeroHalfPI((1.0f - result.ratio) * omega) * invSin;
			float scale1 = Mathf::sinZeroHalfPI(result.ratio * omega) * invSin;

			scale1 = (cosVal >= 0.0f) ? scale1 : -scale1;

			o = scale0 * f1 + scale1 * f2;
		}
		else	// Fallback to linear
			goto Linear;
	}
}

bool AnimationTrack::checkValid() const
{
	MCD_ASSERT(mMutex.isLocked());

	if(subtrackCount() == 0)
		return false;

	// Check that keyframeTimes are positive, unique and in ascending order.
	for(size_t t=0; t<subtrackCount(); ++t) 
	{
		if(subtracks[t].frameCount == 0)
			return false;

		KeyFrames f = const_cast<AnimationTrack*>(this)->getKeyFramesForSubtrack(t);
		float previousTime = f[0].time;
		for(size_t i=1; i<f.size; ++i) {	// Note that we start the index at 1
			if(f[i].time <= previousTime)
				return false;
			previousTime = f[i].time;
		}
	}

	return true;
}

void AnimationTrack::acquireReadLock() const {
	mMutex.lock();
}

void AnimationTrack::releaseReadLock() const {
	mMutex.unlock();
}

void AnimationTrack::acquireWriteLock() const
{
	mMutex.lock();
	mCommitted = false;
}

void AnimationTrack::releaseWriteLock() const
{
	mCommitted = true;

	// Find out the longest sub-track
	mTotalTime = 0;
	for(size_t i=0; i<subtrackCount(); ++i) {
		float t = totalTime(i);
		mTotalTime = t > mTotalTime ? t : mTotalTime;
	}

	mMutex.unlock();
}

bool AnimationTrack::isCommitted() const
{
	MCD_ASSERT(mMutex.isLocked());
	return mCommitted;
}

}	// namespace MCD
