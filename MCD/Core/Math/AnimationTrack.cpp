#include "Pch.h"
#include "AnimationTrack.h"
#include "BasicFunction.h"
#include "Vec4.h"
#include <math.h>	// for fmodf()

namespace MCD {

AnimationClip::AnimationClip(const Path& fileId)
	: Resource(fileId)
	, samples(nullptr, 0)
	, tracks(nullptr, 0)
	, length(0)
	, framerate(30), loopCount(0)
{
}

AnimationClip::~AnimationClip()
{
	::free(samples.getPtr());
	::free(tracks.getPtr());
}

bool AnimationClip::init(const StrideArray<const size_t>& trackFrameCount)
{
	if(trackFrameCount.isEmpty())
		return false;

	::free(samples.getPtr());
	::free(tracks.getPtr());

	// Find out the total sample count of all tracks
	size_t totalFrameCount = 0;
	for(size_t i=0; i<trackFrameCount.size; ++i)
		totalFrameCount += trackFrameCount[i];

	const size_t trackCount = trackFrameCount.size;
	samples = Samples(reinterpret_cast<Sample*>(::malloc(totalFrameCount * sizeof(Sample))), totalFrameCount);
	tracks = Tracks(reinterpret_cast<Track*>(::malloc(trackCount * sizeof(Track))), trackCount);

	::memset(samples.data, 0, samples.sizeInByte());
	::memset(tracks.data, 0, tracks.sizeInByte());

	size_t j = 0;
	for(size_t i=0; i<tracks.size; ++i) {
		const_cast<size_t&>(tracks[i].index) = j;
		j += (const_cast<size_t&>(tracks[i].sampleCount) = trackFrameCount[i]);
		tracks[i].flag = Linear;
	}

	return true;
}

size_t AnimationClip::trackCount() const
{
	return tracks.size;
}

float AnimationClip::lengthForTrack(size_t index) const
{
	if(index < trackCount()) {
		Samples f = const_cast<AnimationClip*>(this)->getSamplesForTrack(index);
		return f[tracks[index].sampleCount - 1].pos;
	}
	return 0;
}

AnimationClip::Samples AnimationClip::getSamplesForTrack(size_t index)
{
	if(index < trackCount())
		return Samples(&samples[tracks[index].index], tracks[index].sampleCount);

	MCD_ASSERT(false && "AnimationClip::getSamplesForTrack out of range");
	return Samples(nullptr, 0);
}

void AnimationClip::interpolate(float pos, const Pose& result, size_t searchHint) const
{
	for(size_t i=0; i<tracks.size; ++i)
		interpolateSingleTrack(pos, length, result[i], i, searchHint);
}

void AnimationClip::interpolateSingleTrack(float trackPos, float totalLen, TrackValue& result, size_t trackIndex, size_t searchHint) const
{
	Samples samples = const_cast<AnimationClip*>(this)->getSamplesForTrack(trackIndex);

	MCD_ASSERT(samples.size > 0);

	// If the animation has only one sample, there is no need to 
	// do any interpolation, simply copy the data.
	if(samples.size == 1) {
		::memcpy(result.v, samples[0].v, sizeof(result.v));
		return;
	}

	{	// Phase 1: Clamp pos within the track's length
		trackPos = Mathf::clamp(trackPos, 0, lengthForTrack(trackIndex));
		MCD_ASSERT(trackPos >= 0);
	}

	size_t idx1, idx2;
	float ratio;	// Ratio between idx1 and idx2

	{	// Phase 2: Find the current and pervious sample index
		size_t curr = (searchHint < samples.size && samples[searchHint].pos < trackPos) ? searchHint : 0; 

		// Scan for a sample with it's pos larger than the current. If none can find, the last sample index is used.
		size_t i = curr;
		for(curr = samples.size - 1; i < samples.size; ++i)
			if(samples[i].pos > trackPos) { curr = i; break; }

		idx2 = (curr == 0) ? 1 : size_t(curr);
		idx1 = idx2 - 1;
	}

	{	// Phase 3: compute the weight between the idx1 and idx2
		const float t1 = samples[idx1].pos;
		const float t2 = samples[idx2].pos;

		MCD_ASSUME(t2 > t1);
		ratio = (trackPos - t1) / (t2 - t1);
	}

	MCD_ASSERT(ratio >= 0);

	// Phase 4: perform interpolation
	const Vec4f& f1 = samples[idx1].cast<const Vec4f>();
	const Vec4f& f2 = samples[idx2].cast<const Vec4f>();
	Vec4f& o = result.cast<Vec4f>();

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
}

bool AnimationClip::checkValid() const
{
	if(trackCount() == 0)
		return false;

	// Check that keyframeTimes are positive, unique and in ascending order.
	for(size_t t=0; t<trackCount(); ++t) 
	{
		if(tracks[t].sampleCount == 0)
			return false;

		Samples f = const_cast<AnimationClip*>(this)->getSamplesForTrack(t);
		float previousPos = f[0].pos;
		if(previousPos != 0)				// Make sure there is always a sample on time = 0
			return false;
		for(size_t i=1; i<f.size; ++i) {	// Note that we start the index at 1
			if(f[i].pos <= previousPos)
				return false;
			previousPos = f[i].pos;
		}
	}

	return true;
}

void AnimationClip::swap(AnimationClip& rhs)
{
	std::swap(samples, rhs.samples);
	std::swap(tracks, rhs.tracks);
	std::swap(length, rhs.length);
	std::swap(framerate, rhs.framerate);
	std::swap(loopCount, rhs.loopCount);
}

}	// namespace MCD
