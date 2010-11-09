#include "Pch.h"
#include "AnimationInstance.h"
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
	realloc(keyIdxHint.getPtr(), 0);
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

void AnimationState::blendResultTo(Pose& accumulatePose, float accumulatedWeight)
{
	MCD_ASSERT(clip->trackCount() == accumulatePose.size);
	if(weight == 0) return;

	const float t = localTime();
	Vec4f dummy; (void)dummy;

	// Allocate the key idx hint if needed
	if(keyIdxHint.size != clip->trackCount()) {
		keyIdxHint.data = (char*)realloc(keyIdxHint.getPtr(), keyIdxHint.cStride * clip->trackCount());
		keyIdxHint.size = clip->trackCount();
		memset(keyIdxHint.data, 0, keyIdxHint.sizeInByte());
	}

	for(size_t i=0; i<accumulatePose.size; ++i) {
		AnimationClip::Sample sample;
		clip->interpolateSingleTrack(t, clip->length, sample, i, keyIdxHint[i]);

		// Handling the quaternion
		if(clip->tracks[i].flag == AnimationClip::Slerp) {
			Quaternionf& q1 = accumulatePose[i].cast<Quaternionf>();
			const Quaternionf& q2 = sample.cast<Quaternionf>();;

			q1 = Quaternionf::slerp(q2, q1, accumulatedWeight/(weight + accumulatedWeight));
			q1 = q1 / q1.length();	// NOTE: Why it still need normalize after slerp?
		}
		else
			accumulatePose[i].cast<Vec4f>() += weight * sample.cast<Vec4f>();
	}
}

AnimationInstance::AnimationInstance()
	: time(0), weightedResult(nullptr, 0), interpolations(nullptr, 0)
{
}

AnimationInstance::~AnimationInstance()
{
	delete[] weightedResult.getPtr();
	delete[] interpolations.getPtr();
}

AnimationInstance::AnimationInstance(const AnimationInstance& rhs)
	: time(rhs.time), weightedResult(nullptr, 0), interpolations(nullptr, 0), mTracks(rhs.mTracks)
{
}

AnimationInstance& AnimationInstance::operator=(const AnimationInstance& rhs)
{
	time = rhs.time;
	mTracks = rhs.mTracks;
	(void)resetInterpolatedResult();
	return *this;
}

void AnimationInstance::update() {}
bool AnimationInstance::isAllTrackCommited() const { return false; }

bool AnimationInstance::resetInterpolatedResult()
{
	delete[] weightedResult.getPtr();
	delete[] interpolations.getPtr();
	const_cast<KeyFrames&>(weightedResult) = KeyFrames(nullptr, 0);
	const_cast<Interpolations&>(interpolations) = Interpolations(nullptr, 0);

	if(mTracks.empty())
		return false;

	const size_t cSubtrackCnt = mTracks[0].track->trackCount();

	MCD_FOREACH(const WeightedTrack& wt, mTracks) {
//		if(!wt.track->isCommitted())
//			return false;

		if(cSubtrackCnt != wt.track->trackCount()) {
			Log::format(
				Log::Warn,
				"Incompatible AnimationClip: subtrack count not matched."
				"Animation will not be updated.");

			return false;
		}
	}

	const_cast<KeyFrames&>(weightedResult) =
		KeyFrames(new KeyFrame[cSubtrackCnt], cSubtrackCnt);

	const_cast<Interpolations&>(interpolations) =
		Interpolations(new Interpolation[cSubtrackCnt], cSubtrackCnt);

	return true;
}
/*
// TODO: Optimize for single tracks (mTracks.size() == 1)?
void AnimationInstance::update()
{
	ScopeRecursiveLock lock(mMutex);

	KeyFrames& result = const_cast<KeyFrames&>(weightedResult);

	if(!result.data && !resetInterpolatedResult())
		return;

	// Zero out weightedResult first
	::memset(&result[0], 0, result.sizeInByte());

	// We cannot use foreach here
	const size_t cTrackCnt = mTracks.size();

	for(size_t i = 0; i < cTrackCnt; ++i) {
		WeightedTrack& wt = mTracks[i];

		if(wt.weight == 0 || !wt.track) continue;

		AnimationClip& t = *wt.track;

		// Assign naturalFramerate to wt.frameRate if it is <= 0
		if(wt.frameRate <= 0)
			wt.frameRate = wt.track->naturalFramerate;

		const float adjustedPos = t.interpolate(time * wt.frameRate, interpolations, wt.loopOverride);

		const size_t subTrackCount = t.trackCount();
		for(size_t j=0; j<subTrackCount; ++j)
			reinterpret_cast<Vec4f&>(result[j]) += wt.weight * reinterpret_cast<Vec4f&>(interpolations[j].v);

		{	// Invoke event callback if necessary
			const size_t triggerWhenEnter = 1;	// Set this to 1 for triggering the event when the frame is enter, otherwise triggered when leaving the frame.
			const size_t lastVirtualFrameIdx = size_t(wt.lastEventPos);
			const size_t currentVirtualFrameIdx = size_t(adjustedPos);

			if(!wt.edgeEvents.empty()) {
				const size_t largestFrameCount = size_t(length());
				for(size_t j=lastVirtualFrameIdx; j!=currentVirtualFrameIdx + triggerWhenEnter;) {
					if(const Event* e = wt.edgeEvents.getEvent(j))
						e->callback(*e);
					++j;
					// Handles the case when the time is loop over.
					j = j > largestFrameCount + triggerWhenEnter ? 0 : j;
				}
			}
			if(!wt.levelEvents.empty()) {
				if(const Event* e = wt.levelEvents.getEvent(currentVirtualFrameIdx))
					e->callback(*e);
			}

			wt.lastEventPos = adjustedPos + triggerWhenEnter;
		}
	}
}

bool AnimationInstance::addTrack(AnimationClip& track, float weight, float framerate, const char* name)
{
	ScopeRecursiveLock lock(mMutex);

	WeightedTrack t = { weight, framerate, -1, 0.0f, name, &track, Events(), Events() };
	mTracks.push_back(t);

	// Destroy the weightedResult and let update() to recreate it,
	delete[] weightedResult.getPtr();
	const_cast<KeyFrames&>(weightedResult) = KeyFrames(nullptr, 0);

	return true;
}

void AnimationInstance::removeTrack(size_t index)
{
	ScopeRecursiveLock lock(mMutex);

	mTracks.erase(mTracks.begin() + index);
	normalizeWeight();
}

void AnimationInstance::normalizeWeight()
{
	ScopeRecursiveLock lock(mMutex);

	float totalWeight = 0;
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		if(t.track)	// Ignore null track
			totalWeight += t.weight;
	}

	if(totalWeight > 0) {
		const float inv = 1.0f / totalWeight;
		MCD_FOREACH(const WeightedTrack& t, mTracks)
			const_cast<WeightedTrack&>(t).weight *= inv;
	}
}

size_t AnimationInstance::trackCount() const
{
	return mTracks.size();
}
*/
size_t AnimationInstance::subtrackCount() const
{
	if(mTracks.empty() || !mTracks[0].track)
		return 0;
	return mTracks[0].track->trackCount();
}
/*
float AnimationInstance::length() const
{
	float longest = 0;
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		if(t.track) {	// Ignore null track
			const float tmp = t.track->length();
			longest = tmp > longest ? tmp : longest;
		}
	}
	return longest;
}

float AnimationInstance::totalTime() const
{
	float longest = 0;
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		if(t.track) {	// Ignore null track
			const float tmp = t.track->length() / t.frameRate;
			longest = tmp > longest ? tmp : longest;
		}
	}
	return longest;
}

AnimationInstance::WeightedTrack* AnimationInstance::getTrack(size_t index)
{
	if(index >= mTracks.size()) return nullptr;
	return &mTracks[index];
}

const AnimationInstance::WeightedTrack* AnimationInstance::getTrack(size_t index) const
{
	return const_cast<AnimationInstance*>(this)->getTrack(index);
}

AnimationInstance::WeightedTrack* AnimationInstance::getTrack(const char* weightedTrackName)
{
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		if(t.name == weightedTrackName)
			return const_cast<WeightedTrack*>(&t);
	}
	return nullptr;
}

const AnimationInstance::WeightedTrack* AnimationInstance::getTrack(const char* weightedTrackName) const
{
	return const_cast<AnimationInstance*>(this)->getTrack(weightedTrackName);
}

int AnimationInstance::getTrackIndex(sal_in_z const char* weightedTrackName) const
{
	for(size_t i=0; i<mTracks.size(); ++i) {
		if(mTracks[i].name == weightedTrackName)
			return int(i);
	}
	return -1;
}

bool AnimationInstance::isAllTrackCommited() const
{
	ScopeRecursiveLock lock(mMutex);
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		t.track->acquireReadLock();
		const bool committed = t.track && t.track->isCommitted();
		t.track->releaseReadLock();
		if(!committed)
			return false;
	}
	return true;
}

size_t AnimationInstance::currentFrame(size_t weightedTrackIndex) const
{
	if(weightedTrackIndex >= interpolations.size)
		return 0;

	return interpolations[weightedTrackIndex].frame1Idx;
}

void AnimationInstance::setTimeByFrameIndex(size_t frameIndex, size_t weightedTrackIndex)
{
	WeightedTrack* t = getTrack(weightedTrackIndex);
	if(!t || !t->track || t->track->keyframes.size == 0)
		return;

	const size_t maxFrameIdx = t->track->keyframes.size - 1;
	time = t->track->keyframes[frameIndex < maxFrameIdx ? frameIndex : maxFrameIdx].pos / t->frameRate;
}
*/
}	// namespace MCD
