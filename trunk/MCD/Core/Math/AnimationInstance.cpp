#include "Pch.h"
#include "AnimationInstance.h"
#include "Vec4.h"
#include "../System/Utility.h"

namespace MCD {

AnimationInstance::AnimationInstance()
	: time(0), interpolatedResult(nullptr, 0)
{
}

AnimationInstance::~AnimationInstance()
{
	delete[] interpolatedResult.getPtr();
}

// TODO: Optimize for single tracks (mTracks.size() == 0)?
void AnimationInstance::update()
{
	// Zero out interpolatedResult first
	::memset(&interpolatedResult[0], 0, interpolatedResult.sizeInByte());

	MCD_FOREACH(const WeightedTrack& wt, mTracks) {
		if(wt.weight == 0 || !wt.track) continue;

		AnimationTrack& t = *wt.track;
		t.update(time);

		for(size_t i=0; i<t.subtrackCount(); ++i)
			reinterpret_cast<Vec4f&>(interpolatedResult[i]) += wt.weight * reinterpret_cast<Vec4f&>(t.interpolatedResult[i]);
	}
}

bool AnimationInstance::addTrack(AnimationTrack& track, float weight)
{
	if(track.subtrackCount() == 0)
		return false;

	if(!mTracks.empty() && subtrackCount() != track.subtrackCount())
		return false;

	WeightedTrack t = { weight, &track };
	mTracks.push_back(t);

	delete[] interpolatedResult.getPtr();
	interpolatedResult = AnimationTrack::KeyFrames(
		new AnimationTrack::KeyFrame[track.subtrackCount()], track.subtrackCount()
	);

	return true;
}

void AnimationInstance::removeTrack(size_t index)
{
	mTracks.erase(mTracks.begin() + index);
	normalizeWeight();
}

void AnimationInstance::normalizeWeight()
{
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

size_t AnimationInstance::trackCount() const {
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

const AnimationInstance::WeightedTrack* AnimationInstance::getTrack(size_t index) const
{
	return const_cast<AnimationInstance*>(this)->getTrack(index);
}

}	// namespace MCD
