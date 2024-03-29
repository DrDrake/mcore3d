#include "Pch.h"
#include "AnimationClip.h"
#include "BasicFunction.h"
#include "Quaternion.h"
#include "Vec4.h"
#include <math.h>	// for fmodf()

namespace MCD {

AnimationClip::AnimationClip(const Path& fileId)
	: Resource(fileId)
	, keyBuffer(nullptr, 0)
	, tracks(nullptr, 0)
	, length(0)
	, framerate(30), loopCount(0)
{
}

AnimationClip::~AnimationClip()
{
	::free(keyBuffer.getPtr());
	::free(tracks.getPtr());
}

void AnimationClip::Sample::blend(float t, const Sample& s1, const Sample& s2)
{
	MCD_ASSERT(s1.flag == s2.flag);
	if(AnimationClip::Linear == s1.flag)
		v = s1.v + t * (s2.v - s1.v);
	else if(AnimationClip::Slerp == s1.flag) {
		Quaternionf& q = cast<Quaternionf>();
		q = Quaternionf::slerp(s1.cast<Quaternionf>(), s2.cast<Quaternionf>(), t);
	}
	else if(AnimationClip::Step == s1.flag)
		v = s1.v;
	else {	MCD_ASSERT(false); }
}

bool AnimationClip::init(const StrideArray<size_t>& trackFrameCount)
{
	if(trackFrameCount.isEmpty())
		return false;

	::free(keyBuffer.getPtr());
	::free(tracks.getPtr());

	// Find out the total key count of all tracks
	size_t totalFrameCount = 0;
	for(size_t i=0; i<trackFrameCount.size; ++i)
		totalFrameCount += trackFrameCount[i];

	const size_t trackCount = trackFrameCount.size;
	keyBuffer = Keys(reinterpret_cast<Key*>(::malloc(totalFrameCount * sizeof(Key))), totalFrameCount);
	tracks = Tracks(reinterpret_cast<Track*>(::malloc(trackCount * sizeof(Track))), trackCount);

	::memset(keyBuffer.data, 0, keyBuffer.sizeInByte());
	::memset(tracks.data, 0, tracks.sizeInByte());

	size_t j = 0;
	for(size_t i=0; i<tracks.size; ++i) {
		const_cast<size_t&>(tracks[i].index) = j;
		j += (const_cast<size_t&>(tracks[i].keyCount) = trackFrameCount[i]);
		tracks[i].flag = Linear;
	}

	return true;
}

void AnimationClip::addTrack(size_t keyCount, Flags flag)
{
	MCD_ASSERT(keyCount > 0);
	tracks.size++;
	tracks.data = (char*)::realloc(tracks.data, tracks.sizeInByte());

	tracks[tracks.size-1].flag = flag;
	const_cast<size_t&>(tracks[tracks.size-1].index) = keyBuffer.size;
	const_cast<size_t&>(tracks[tracks.size-1].keyCount) = keyCount;

	keyBuffer.size += keyCount;
	keyBuffer.data = (char*)::realloc(keyBuffer.data, keyBuffer.sizeInByte());
}

size_t AnimationClip::trackCount() const
{
	return tracks.size;
}

float AnimationClip::lengthForTrack(size_t index) const
{
	if(index < trackCount()) {
		Keys k = const_cast<AnimationClip*>(this)->getKeysForTrack(index);
		return k[tracks[index].keyCount - 1].pos;
	}
	return 0;
}

AnimationClip::Keys AnimationClip::getKeysForTrack(size_t index)
{
	if(index < trackCount())
		return Keys(&keyBuffer[tracks[index].index], tracks[index].keyCount);

	MCD_ASSERT(false && "AnimationClip::getKeysForTrack out of range");
	return Keys(nullptr, 0);
}

void AnimationClip::sample(float pos, const Pose& result, const KeyIdxHint& hint) const
{
	if(hint.size == 0) for(size_t i=0; i<tracks.size; ++i)
		sampleSingleTrack(pos, length, result[i], i, 0);
	else for(size_t i=0; i<tracks.size; ++i)
		hint[i] = (uint16_t)sampleSingleTrack(pos, length, result[i], i, hint[i]);
}

size_t AnimationClip::sampleSingleTrack(float trackPos, float totalLen, Sample& result, size_t trackIndex, size_t keySearchHint) const
{
	Keys keys = const_cast<AnimationClip*>(this)->getKeysForTrack(trackIndex);

	MCD_ASSERT(keys.size > 0);

	result.flag = tracks[trackIndex].flag;

	// If the animation has only one key, there is no need to 
	// do any interpolation, simply copy the data.
	if(keys.size == 1) {
		::memcpy(&result.v, &keys[0].v, sizeof(result.v));
		return 0;
	}

	{	// Phase 1: Clamp pos within the track's length
		trackPos = Mathf::clamp(trackPos, 0, lengthForTrack(trackIndex));
		MCD_ASSERT(trackPos >= 0);
	}

	size_t idx1, idx2;
	float ratio;	// Ratio between idx1 and idx2

	{	// Phase 2: Find the current and pervious key index
		size_t curr = (keySearchHint < keys.size && keys[keySearchHint].pos < trackPos) ? keySearchHint : 0; 

		// Scan for a key with it's pos larger than the current. If none can find, the last key index is used.
		size_t i = curr;
		for(curr = keys.size - 1; i < keys.size; ++i)
			if(keys[i].pos > trackPos) { curr = i; break; }

		idx2 = (curr == 0) ? 1 : size_t(curr);
		idx1 = idx2 - 1;
	}

	{	// Phase 3: compute the weight between the idx1 and idx2
		const float t1 = keys[idx1].pos;
		const float t2 = keys[idx2].pos;

		MCD_ASSUME(t2 > t1);
		ratio = (trackPos - t1) / (t2 - t1);
	}

	MCD_ASSERT(ratio >= 0);

	// Short cut optimization
	if(ratio == 0) {
		::memcpy(&result.v, &keys[idx1].v, sizeof(result.v));
		return idx1;
	}

	// Phase 4: perform interpolation
	const Vec4f& f1 = keys[idx1].v;
	const Vec4f& f2 = keys[idx2].v;
	Vec4f& o = result.v;

	const Flags flag = tracks[trackIndex].flag;
	if(flag == Linear)
	{
	Linear:
		o = f1 + ratio * (f2 - f1);
	}
	else if(flag == Slerp)
	{
		// Refernece: From ID software, "Slerping Clock Cycles"
		const float cosVal = f1 % f2;
		const float absCosVal = fabsf(cosVal);
		if((1.0f - absCosVal) > 1e-6f)
		{
			// Standard case (slerp)
			const float sinSqr = 1.0f - absCosVal * absCosVal;
			const float invSin = 1.0f / sqrtf(sinSqr);
			const float omega = Mathf::aTanPositive(sinSqr * invSin, absCosVal);
			const float scale0 = Mathf::sinZeroHalfPI((1.0f - ratio) * omega) * invSin;
			float scale1 = Mathf::sinZeroHalfPI(ratio * omega) * invSin;

			scale1 = (cosVal >= 0.0f) ? scale1 : -scale1;

			o = scale0 * f1 + scale1 * f2;
		}
		else	// Fallback to linear
			goto Linear;
	}
	else if(flag == Step)
		o = f1;

	return idx1;
}

bool AnimationClip::checkValid() const
{
	if(trackCount() == 0)
		return false;

	// Check that keyframeTimes are positive, unique and in ascending order.
	for(size_t t=0; t<trackCount(); ++t) 
	{
		if(tracks[t].keyCount == 0)
			return false;

		Keys k = const_cast<AnimationClip*>(this)->getKeysForTrack(t);
		float previousPos = k[0].pos;
		if(previousPos != 0)				// Make sure there is always a key on time = 0
			return false;
		for(size_t i=1; i<k.size; ++i) {	// Note that we start the index at 1
			if(k[i].pos <= previousPos)
				return false;
			previousPos = k[i].pos;
		}
	}

	return true;
}

void AnimationClip::swap(AnimationClip& rhs)
{
	std::swap(keyBuffer, rhs.keyBuffer);
	std::swap(tracks, rhs.tracks);
	std::swap(length, rhs.length);
	std::swap(framerate, rhs.framerate);
	std::swap(loopCount, rhs.loopCount);
}

bool AnimationClip::createDifferenceClip(AnimationClip& master, AnimationClip& target)
{
	// Perform some compatibility tests first
	const size_t trackCount = master.trackCount();
	if(master.trackCount() != target.trackCount()) return false;
	for(size_t i=0; i<trackCount; ++i) {
		if(master.tracks[i].flag != target.tracks[i].flag)
			return false;
	}

	length = target.length;
	framerate = target.framerate;
	loopCount = target.loopCount;

	{	// Init the diff clip with the same number of keys as the target
		StrideArray<size_t> trackKeyCount(new size_t[trackCount], trackCount);
		for(size_t i=0; i<trackCount; ++i)
			trackKeyCount[i] = target.getKeysForTrack(i).size;
		bool ok = init(trackKeyCount);
		delete[] trackKeyCount.getPtr();
		if(!ok) return false;
	}

	// Assign key position
	for(size_t i=0; i<trackCount; ++i) {
		tracks[i].flag = target.tracks[i].flag;

		Keys k1 = getKeysForTrack(i);
		Keys k2 = target.getKeysForTrack(i);
		for(size_t j=0; j<k1.size; ++j)
			k1[j].pos = k2[j].pos;
	}

	// Sample both master and target, calculate the difference
	for(size_t i=0; i<trackCount; ++i)
	{
		Keys k1 = getKeysForTrack(i);
		Keys k2 = target.getKeysForTrack(i);	// For assertion use only
		MCD_ASSERT(k1.size == k2.size);

		for(size_t j=0; j<k1.size; ++j)
		{
			MCD_ASSERT(k1[j].pos == k2[j].pos);
			Sample s1, s2;
			master.sampleSingleTrack(k1[j].pos, master.length, s1, i);
			target.sampleSingleTrack(k1[j].pos, target.length, s2, i);

			switch(s1.flag) {
			case Linear:
			case Step:
				k1[j].v = s2.v - s1.v;
				break;
			case Slerp:
				// For quaternion:
				// target = master * master.inverse() * target
				// target = master * diff where diff = master.inverse() * target
				k1[j].cast<Quaternionf>() = s1.cast<Quaternionf>().inverse() * s2.cast<Quaternionf>();
				break;
			default: MCD_ASSERT(false);
			}
		}
	}

	return true;
}

}	// namespace MCD
