#include "Pch.h"
#include "AnimationTrack.h"
#include "BasicFunction.h"
#include "Vec4.h"
#include "../System/MemoryProfiler.h"
#include <math.h>	// for fmodf()

namespace MCD {

AnimationTrack::AnimationTrack(const Path& fileId)
	: Resource(fileId)
	, keyframes(nullptr, 0)
	, keyframeTimes(nullptr, 0)
	, subtrackFlags(nullptr, 0)
	, loop(true), mCommitted(false), frame1Idx(0), frame2Idx(1), ratio(0)
	, interpolatedResult(nullptr, 0)
{
}

AnimationTrack::~AnimationTrack()
{
	delete[] keyframes.getPtr();
	delete[] keyframeTimes.getPtr();
	delete[] subtrackFlags.getPtr();
	delete[] interpolatedResult.getPtr();
}

AnimationTrack::KeyFrames AnimationTrack::getKeyFramesForSubtrack(size_t index)
{
	MCD_ASSERT(mMutex.isLocked());

	if(index < subtrackCount())
		return KeyFrames(&keyframes[index * keyframeCount()], keyframeCount());

	MCD_ASSERT(false && "AnimationTrack::getKeyFramesForSubtrack out of range");
	return KeyFrames(nullptr, 0);
}

float AnimationTrack::totalTime() const
{
	MCD_ASSERT(mMutex.isLocked());

	return keyframeTimes[keyframeCount() - 1];
}

float AnimationTrack::currentTime() const
{
	MCD_ASSERT(mMutex.isLocked());

	const float t1 = keyframeTimes[frame1Idx];
	const float t2 = keyframeTimes[frame2Idx];
	return t1 + ratio * (t2 - t1);
}

bool AnimationTrack::init(size_t keyFrameCnt, size_t subtrackCnt)
{
	if(keyFrameCnt < 1 || subtrackCnt < 1)
		return false;

	MCD_ASSERT(mMutex.isLocked());

	if(isCommitted())
		return false;

	delete[] keyframes.getPtr();
	delete[] keyframeTimes.getPtr();
	delete[] subtrackFlags.getPtr();
	delete[] interpolatedResult.getPtr();

	MemoryProfiler::Scope scope("AnimationTrack::init");
	keyframes = KeyFrames(new KeyFrame[keyFrameCnt * subtrackCnt], keyFrameCnt * subtrackCnt);
	keyframeTimes = FixStrideArray<float>(new float[keyFrameCnt], keyFrameCnt);
	subtrackFlags = FixStrideArray<Flags>(new Flags[subtrackCnt], subtrackCnt);
	interpolatedResult = KeyFrames(new KeyFrame[subtrackCnt], subtrackCnt);

	::memset(keyframes.data, 0, keyframes.sizeInByte());
	::memset(keyframeTimes.data, 0, keyframeTimes.sizeInByte());
	::memset(subtrackFlags.data, 0, subtrackFlags.sizeInByte());
	::memset(interpolatedResult.data, 0, interpolatedResult.sizeInByte());

	for(size_t i=0; i<subtrackFlags.size; ++i)
		subtrackFlags[i] = Linear;

	return true;
}

void AnimationTrack::update(float currentTime)
{
	ScopeLock lock(mMutex);
	return updateNoLock(currentTime);
}

void AnimationTrack::updateNoLock(float currentTime)
{
	MCD_ASSERT(mMutex.isLocked());

	if(!mCommitted)
		return;

	// If the animation has only one frame, there is no need to 
	// do any interpolation, simply copy the data.
	if(keyframeTimes.size < 2) {
		for(size_t i=0; i<subtrackFlags.size; ++i)
			interpolatedResult[i] = getKeyFramesForSubtrack(i)[0];
		return;
	}

	// Phase 1: find the wrapped version of currentTime
	if(loop)
		currentTime = ::fmodf(currentTime, totalTime());
	else
		currentTime = Mathf::clamp(currentTime, currentTime, totalTime());

	// Phase 2: for each track, find t_current and t_pervious
	size_t curr = keyframeTimes[frame1Idx] < currentTime ? frame1Idx : 0;

	for(size_t i=curr; i < keyframeCount(); ++i)
		if(keyframeTimes[i] >= currentTime) { curr = i; break; }

	frame2Idx = (curr == 0) ? 1 : size_t(curr);
	frame1Idx = frame2Idx - 1;

	// Phase 3: compute the weight between the frame1Idx and frame2Idx for each track
	const float t1 = keyframeTimes[frame1Idx];
	const float t2 = keyframeTimes[frame2Idx];

	MCD_ASSUME(t2 > t1);
	ratio = (currentTime - t1) / (t2 - t1);

	// Phase 4: perform interpolation for each sub-track
	for(size_t i=0; i<subtrackFlags.size; ++i)
	{
		const KeyFrames k = getKeyFramesForSubtrack(i);
		const Vec4f& f1 = reinterpret_cast<const Vec4f&>(k[frame1Idx]);
		const Vec4f& f2 = reinterpret_cast<const Vec4f&>(k[frame2Idx]);
		Vec4f& o = reinterpret_cast<Vec4f&>(interpolatedResult[i]);

		if(subtrackFlags[i] == Linear)
		{
		Linear:
			o = f1 + ratio * (f2 - f1);
		}
		else if(subtrackFlags[i] == Slerp)
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
				const float scale0 = Mathf::sinZeroHalfPI((1.0f - ratio) * omega) * invSin;
				float scale1 = Mathf::sinZeroHalfPI(ratio * omega) * invSin;

				scale1 = (cosVal >= 0.0f) ? scale1 : -scale1;

				o = scale0 * f1 + scale1 * f2;
			}
			else	// Fallback to linear
				goto Linear;
		}
	}
}

bool AnimationTrack::checkValid() const
{
	MCD_ASSERT(mMutex.isLocked());

	if(keyframeCount() < 1 || subtrackCount() < 1)
		return false;

	{	// Check that keyframeTimes are positive, unique and in ascending order.
		float previousTime = keyframeTimes[0];
		for(size_t i=1; i<keyframeTimes.size; ++i) {	// Note that we start the index at 1
			if(keyframeTimes[i] <= previousTime)
				return false;
			previousTime = keyframeTimes[i];
		}
	}

	return true;
}

void AnimationTrack::acquireReadLock()
{
	mMutex.lock();
}

void AnimationTrack::releaseReadLock()
{
	mMutex.unlock();
}

void AnimationTrack::acquireWriteLock()
{
	mMutex.lock();
	mCommitted = false;
}

void AnimationTrack::releaseWriteLock()
{
	mCommitted = true;
	mMutex.unlock();
}

bool AnimationTrack::isCommitted() const {
	MCD_ASSERT(mMutex.isLocked());
	return mCommitted;
}

}	// namespace MCD
