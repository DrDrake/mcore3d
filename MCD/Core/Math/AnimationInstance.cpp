#include "Pch.h"
#include "AnimationInstance.h"
#include "Vec4.h"
#include "../System/Utility.h"
#include "../System/Log.h"

namespace MCD {

AnimationInstance::AnimationInstance()
	: time(0), interpolatedResult(nullptr, 0)
{
}

AnimationInstance::~AnimationInstance()
{
	delete[] interpolatedResult.getPtr();
}

AnimationInstance::AnimationInstance(const AnimationInstance& rhs)
	: time(rhs.time), interpolatedResult(nullptr, 0), mTracks(rhs.mTracks)
{
}

AnimationInstance& AnimationInstance::operator=(const AnimationInstance& rhs)
{
	time = rhs.time;
	mTracks = rhs.mTracks;
	(void)resetInterpolatedResult();
	return *this;
}

bool AnimationInstance::resetInterpolatedResult()
{
	delete[] interpolatedResult.getPtr();
	const_cast<AnimationTrack::KeyFrames&>(interpolatedResult) = AnimationTrack::KeyFrames(nullptr, 0);

	if(mTracks.empty())
		return false;

	mTracks[0].track->acquireReadLock();
	const size_t cSubtrackCnt = mTracks[0].track->subtrackCount();
	mTracks[0].track->releaseReadLock();

	MCD_FOREACH(const WeightedTrack& wt, mTracks) {
		wt.track->acquireReadLock();

		if(!wt.track->isCommitted()) {
			wt.track->releaseReadLock();
			return false;
		}

		if(cSubtrackCnt != wt.track->subtrackCount()) {
			Log::format(
				Log::Warn,
				L"Incompatible AnimationTrack: subtrack count not matched."
				L"Animation will not be updated.");

			wt.track->releaseReadLock();
			return false;
		}

		wt.track->releaseReadLock();
	}

	const_cast<AnimationTrack::KeyFrames&>(interpolatedResult) =
		AnimationTrack::KeyFrames(new AnimationTrack::KeyFrame[cSubtrackCnt], cSubtrackCnt);

	return true;
}

// TODO: Optimize for single tracks (mTracks.size() == 0)?
void AnimationInstance::update()
{
	ScopeRecursiveLock lock(mMutex);

	AnimationTrack::KeyFrames& result = const_cast<AnimationTrack::KeyFrames&>(interpolatedResult);

	if(!result.data && !resetInterpolatedResult())
		return;

	// Zero out interpolatedResult first
	::memset(&result[0], 0, result.sizeInByte());

	// We cannot use foreach here
	const size_t cTrackCnt = mTracks.size();

	for(size_t i = 0; i < cTrackCnt; ++i) {

		WeightedTrack& wt = mTracks[i];

		if(wt.weight == 0 || !wt.track) continue;

		AnimationTrack& t = *wt.track;

		t.acquireReadLock();

		// Assign naturalFramerate to wt.frameRate if it is <= 0
		if(wt.frameRate <= 0)
			wt.frameRate = wt.track->naturalFramerate;

		t.updateNoLock(time * wt.frameRate);

		for(size_t j=0; j<t.subtrackCount(); ++j)
			reinterpret_cast<Vec4f&>(result[j]) += wt.weight * reinterpret_cast<Vec4f&>(t.interpolatedResult[j]);

		t.releaseReadLock();
	}
}

bool AnimationInstance::addTrack(AnimationTrack& track, float weight, float framerate)
{
	ScopeRecursiveLock lock(mMutex);

	WeightedTrack t = { weight, framerate, &track };
	mTracks.push_back(t);

	// Destroy the interpolatedResult and let update() to recreate it,
	delete[] interpolatedResult.getPtr();
	const_cast<AnimationTrack::KeyFrames&>(interpolatedResult) = AnimationTrack::KeyFrames(nullptr, 0);

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

size_t AnimationInstance::subtrackCount() const
{
	if(mTracks.empty() || !mTracks[0].track)
		return 0;
	return mTracks[0].track->subtrackCount();
}

AnimationInstance::WeightedTrack* AnimationInstance::getTrack(size_t index)
{
	if(index < mTracks.size()) return nullptr;
	return &mTracks[index];
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

const AnimationInstance::WeightedTrack* AnimationInstance::getTrack(size_t index) const
{
	return const_cast<AnimationInstance*>(this)->getTrack(index);
}

}	// namespace MCD
