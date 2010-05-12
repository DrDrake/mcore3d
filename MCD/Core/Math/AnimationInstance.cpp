#include "Pch.h"
#include "AnimationInstance.h"
#include "Vec4.h"
#include "../System/Utility.h"
#include "../System/Log.h"

namespace MCD {

AnimationInstance::Events::Events() : callback(nullptr), destroyData(nullptr) {}

AnimationInstance::Events::~Events() {
	clear();
}

AnimationInstance::Events::Events(const Events& rhs)
{
	callback = rhs.callback;
	destroyData = rhs.destroyData;
}

AnimationInstance::Events& AnimationInstance::Events::operator=(const Events& rhs) {
	// Do nothing
	return *this;
}

AnimationInstance::Event* AnimationInstance::Events::setEvent(size_t virtualFrameIdx, void* data)
{
	// Search for any existing index
	iterator i;
	for(i=begin(); i!=end(); ++i)
		if(i->virtualFrameIdx == virtualFrameIdx)
			break;

	// Adding new value
	if(data && i == end()) {
		// Perform insertion sort: search for any existing index that has
		// the value just larger than the input frame index
		for(i=begin(); i!=end(); ++i)
			if(i->virtualFrameIdx > virtualFrameIdx)
				break;

		Event e = { virtualFrameIdx, data, callback, destroyData };
		return &*(insert(i, e));
	}

	// Update value
	if(data && i != end()) {
		if(i->destroyData) i->destroyData(i->data);
		const_cast<void*&>(i->data) = data;
		return &(*i);
	}

	// Removal
	if(i != end()) {
		if(i->destroyData) i->destroyData(i->data);
		erase(i);
		return nullptr;
	}

	return nullptr;
}

AnimationInstance::Event* AnimationInstance::Events::getEvent(size_t virtualFrameIdx) const
{
	for(const_iterator i=begin(); i!=end(); ++i)
		if(i->virtualFrameIdx == virtualFrameIdx)
			return &const_cast<Event&>(*i);

	return nullptr;
}

bool AnimationInstance::Events::empty() const {
	return std::vector<Event>::empty();
}

void AnimationInstance::Events::clear()
{
	for(iterator i=begin(); i!=end(); ++i)
		if(i->destroyData)
			i->destroyData(i->data);
	std::vector<Event>::clear();
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

bool AnimationInstance::resetInterpolatedResult()
{
	delete[] weightedResult.getPtr();
	delete[] interpolations.getPtr();
	const_cast<KeyFrames&>(weightedResult) = KeyFrames(nullptr, 0);
	const_cast<Interpolations&>(interpolations) = Interpolations(nullptr, 0);

	if(mTracks.empty())
		return false;

	const size_t cSubtrackCnt = mTracks[0].track->subtrackCount();

	MCD_FOREACH(const WeightedTrack& wt, mTracks) {
		AnimationTrack::ScopedReadLock readLock(*wt.track);

		if(!wt.track->isCommitted())
			return false;

		if(cSubtrackCnt != wt.track->subtrackCount()) {
			Log::format(
				Log::Warn,
				"Incompatible AnimationTrack: subtrack count not matched."
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

		AnimationTrack& t = *wt.track;

		// Assign naturalFramerate to wt.frameRate if it is <= 0
		if(wt.frameRate <= 0)
			wt.frameRate = wt.track->naturalFramerate;

		const float adjustedTime = t.interpolate(time * wt.frameRate, interpolations, wt.loopOverride);

		for(size_t j=0; j<t.subtrackCount(); ++j)
			reinterpret_cast<Vec4f&>(result[j]) += wt.weight * reinterpret_cast<Vec4f&>(interpolations[j].v);

		{	// Invoke event callback if necessary
			const size_t triggerWhenEnter = 1;	// Set this to 1 for triggering the event when the frame is enter, otherwise triggered when leaving the frame.
			const size_t lastVirtualFrameIdx = size_t(wt.lastEventTime);
			const size_t currentVirtualFrameIdx = size_t(adjustedTime);

			if(!wt.edgeEvents.empty()) {
				const size_t largestFrameCount = size_t(totalTime());
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

			wt.lastEventTime = adjustedTime + triggerWhenEnter;
		}
	}
}

bool AnimationInstance::addTrack(AnimationTrack& track, float weight, float framerate, const char* name)
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

size_t AnimationInstance::subtrackCount() const
{
	if(mTracks.empty() || !mTracks[0].track)
		return 0;
	return mTracks[0].track->subtrackCount();
}

float AnimationInstance::totalTime() const
{
	float longest = 0;
	MCD_FOREACH(const WeightedTrack& t, mTracks) {
		if(t.track) {	// Ignore null track
			float tmp = t.track->totalTime();
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
			return i;
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
	time = t->track->keyframes[frameIndex < maxFrameIdx ? frameIndex : maxFrameIdx].time;
}

}	// namespace MCD
