#include "Pch.h"
#include "AnimationTrack.h"
#include "BasicFunction.h"
#include "Vec4.h"
#include "../System/MemoryProfiler.h"
#include <math.h>	// for fmodf()

namespace MCD {

AnimationTrack::AnimationTrack(size_t keyFrameCnt, size_t subtrackCnt)
	: keyframes(nullptr, 0)
	, keyframeTimes(nullptr, 0)
	, subtrackFlags(nullptr, 0)
	, loop(true), frame1Idx(0), frame2Idx(1), ratio(0)
	, interpolatedResult(nullptr, 0)
{
	if(keyFrameCnt < 2) keyFrameCnt = 2;
	if(subtrackCnt < 1) subtrackCnt = 1;

	MemoryProfiler::Scope scope("AnimationTrack::AnimationTrack");
	keyframes = KeyFrames(new KeyFrame[keyFrameCnt * subtrackCnt], keyFrameCnt * subtrackCnt);
	keyframeTimes = FixStrideArray<float>(new float[keyFrameCnt], keyFrameCnt);
	subtrackFlags = FixStrideArray<Flags>(new Flags[subtrackCnt], subtrackCnt);
	interpolatedResult = KeyFrames(new KeyFrame[subtrackCnt], subtrackCnt);

	for(size_t i=0; i<subtrackFlags.size; ++i)
		subtrackFlags[i] = Linear;
}

AnimationTrack::~AnimationTrack()
{
	delete[] &keyframes[0];
	delete[] &keyframeTimes[0];
	delete[] &subtrackFlags[0];
	delete[] &interpolatedResult[0];
}

AnimationTrack::KeyFrames AnimationTrack::getKeyFramesForSubtrack(size_t index)
{
	if(index < subtrackCount())
		return KeyFrames(&keyframes[index * keyframeCount()], keyframeCount());

	MCD_ASSERT(false && "AnimationTrack::getKeyFramesForSubtrack out of range");
	return KeyFrames(nullptr, 0);
}

float AnimationTrack::totalTime() const
{
	return keyframeTimes[keyframeCount() - 1];
}

float AnimationTrack::currentTime() const
{
	const float t1 = keyframeTimes[frame1Idx];
	const float t2 = keyframeTimes[frame2Idx];
	return t1 + ratio * (t2 - t1);
}

void AnimationTrack::update(float currentTime)
{
	// Phase 1: find the wrapped version of currentTime
	if(loop)
		currentTime = ::fmodf(currentTime, totalTime());
	else
		currentTime = Mathf::clamp(currentTime, currentTime, totalTime());

	// Phase 2: for each track, find t_current and t_pervious
	int curr = -1;

	// For most cases the parameter currentTime is increasing, so as an optimization
	// we start the search from the cached frame index.
	for(size_t i=frame2Idx; i < keyframeCount(); ++i)
		if(keyframeTimes[i] >= currentTime) { curr = i; break; }

	// If none is found, start the search from the beginning.
	if(curr == int(keyframeCount() - 1))
		for(size_t i=0; i < frame2Idx; ++i)
			if(keyframeTimes[i] >= currentTime) { curr = i; break; }

	MCD_ASSERT(curr != -1);

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
	if(keyframeCount() < 2 || subtrackCount() < 1)
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

}	// namespace MCD
