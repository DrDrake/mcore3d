#include "Pch.h"
#include "AnimationState.h"
#include "BasicFunction.h"
#include "Vec4.h"
#include "Quaternion.h"
#include "../System/Utility.h"
#include "../System/Log.h"
#include <limits>

namespace MCD {

AnimationState::AnimationState()
	: weight(1), rate(1)
	, loopCountOverride(-1)
	, worldTime(0), worldRefTime(0)
	, keyIdxHint(nullptr, 0)
{
}

AnimationState::~AnimationState()
{
	free(keyIdxHint.getPtr());
}

AnimationState::AnimationState(const AnimationState& rhs)
	: name(rhs.name)
	, weight(rhs.weight), rate(rhs.rate)
	, loopCountOverride(rhs.loopCountOverride)
	, worldTime(rhs.worldTime), worldRefTime(rhs.worldRefTime)
	, keyIdxHint(nullptr, 0)
	, clip(rhs.clip)
{
}

int AnimationState::loopCount() const
{
	return loopCountOverride > 0 ? loopCountOverride : int(clip->loopCount);
}

float AnimationState::localTime() const
{
	const int loop = loopCount();
	const float len = clip->length / clip->framerate;
	const float clampLen = loop == 0 ? std::numeric_limits<float>::max() : len * loop;
	float t = fabs(rate) * (worldTime - worldRefTime);
	MCD_ASSERT("Make sure AnimationClip has non-zero length" && len > 0);
	t = Mathf::clamp(t, 0, clampLen);		// Handle looping
	t = t == clampLen ? len : fmod(t, len);	// Handle looping
	t = rate >= 0 ? t : len - t;			// Handle negative playback rate
	MCD_ASSERT(t >= 0 && t <= len);
	return t;
}

float AnimationState::worldEndTime() const
{
	if(rate == 0) return worldRefTime;

	const int loop = loopCount();
	if(loop == 0) return std::numeric_limits<float>::max();

	const float len = clip->length / clip->framerate * loop;
	return worldRefTime + len / fabs(rate);
}

bool AnimationState::ended() const
{
	return worldTime >= worldEndTime();
}

void AnimationState::assignTo(const Pose& pose)
{
	MCD_ASSERT(clip->trackCount() == pose.size);
	const float t = localTime() * clip->framerate;
	allocateIdxHint();

	for(size_t i=0; i<pose.size; ++i) {
		keyIdxHint[i] = (uint16_t)clip->sampleSingleTrack(t, clip->length, pose[i], i, keyIdxHint[i]);
		pose[i].flag = clip->tracks[i].flag;
	}
}

float AnimationState::blendResultTo(const Pose& accumulatePose, float accumulatedWeight)
{
	MCD_ASSERT(clip->trackCount() == accumulatePose.size);
	const float newWeight = weight + accumulatedWeight;
	if(weight == 0) return newWeight;

	const float t = localTime() * clip->framerate;
	Vec4f dummy; (void)dummy;

	allocateIdxHint();

	for(size_t i=0; i<accumulatePose.size; ++i)
	{
		AnimationClip::Sample sample;
		keyIdxHint[i] = (uint16_t)clip->sampleSingleTrack(t, clip->length, sample, i, keyIdxHint[i]);
		accumulatePose[i].flag = sample.flag;

		// Handling the quaternion
		if(clip->tracks[i].flag == AnimationClip::Slerp) {
			Quaternionf& q1 = accumulatePose[i].cast<Quaternionf>();
			const Quaternionf& q2 = sample.cast<Quaternionf>();

			if(accumulatedWeight > 0) {
				q1 = Quaternionf::slerp(q2, q1, accumulatedWeight/newWeight);
				q1 = q1 / q1.length();	// NOTE: Why it still need normalize after slerp?
			}
			else
				q1 = q2;
		}
		else
			accumulatePose[i].cast<Vec4f>() += weight * sample.cast<Vec4f>();
	}

	return newWeight;
}

void AnimationState::allocateIdxHint()
{
	if(keyIdxHint.size != clip->trackCount()) {
		keyIdxHint.data = (char*)realloc(keyIdxHint.getPtr(), keyIdxHint.stride() * clip->trackCount());
		keyIdxHint.size = clip->trackCount();
		memset(keyIdxHint.data, 0, keyIdxHint.sizeInByte());
	}
}

}	// namespace MCD
