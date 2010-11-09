#ifndef __MCD_CORE_MATH_ANIMATIONTRACK__
#define __MCD_CORE_MATH_ANIMATIONTRACK__

#include "Vec4.h"
#include "../System/Array.h"
#include "../System/Resource.h"

namespace MCD {

/*!	A kind of Resource which stores animation data.
	\note The memory layout of the samples is:
	track1Sample1, track1Sample2, track2Sample1, track2Sample2, ...

	Example:
	\code
	AnimationClipPtr clip = new AnimationClip(L"clipName");

	{	clip->init(2, 1);	// One track with 2 frame
		AnimationClip::Samples samples = track->getSamplesForTrack(0);

		// Fill the sample's position
		samples[0].pos = 0;
		samples[1].pos = 1;

		// Fill the attribute
		reinterpret_cast<Vec4f&>(samples[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(samples[1]) = Vec4f(2);
	}

	// For each frame:
	track->acquireReadLock();
	track->updateNoLock(currentTrackPos);

	// Get the interpolated attributes and do what ever you need.
	const Vec4f& pos = reinterpret_cast<const Vec4f&>(track->interpolatedResult[0]);
	track->releaseReadLock();
	\endcode
 */
class MCD_CORE_API AnimationClip : public Resource
{
public:
	explicit AnimationClip(const Path& fileId);

// Inner types
	enum Flags
	{
		Linear	= 1,
		Slerp	= 2,
		Step	= 3,
	};	// Flags

	//! We use a float 4 array to act as a generic datatype for an animated attribute.
	struct Sample
	{
		float v[4];
		float pos;			//!< The position where the sample is taken (float is used instead of integer to reduce type casting, which may be expensive)

		template<class T> T& cast() { return *reinterpret_cast<T*>(v); }
		template<class T> const T& cast() const { return *reinterpret_cast<const T*>(v); }
	};	// Sample

	typedef FixStrideArray<Sample> Samples;

	struct Track
	{
		const size_t index;			//!< Index to the \em samples member variable. Assigned by init(), don't modify it.
		const size_t sampleCount;	//!< Number of sample for this track.
		Flags flag;
	};	// Track

	typedef FixStrideArray<Track> Tracks;

	struct TrackValue
	{
		float v[4];
		size_t searchHint;	//!< Index of sample that just before the current position, aid sample search for a given time pos.

		template<class T> T& cast() { return *reinterpret_cast<T*>(v); }
		template<class T> const T& cast() const { return *reinterpret_cast<const T*>(v); }
	};	// TrackValue

	typedef FixStrideArray<TrackValue> Pose;

	struct Interpolation
	{
		float v[4];
		size_t idx1;	//!< Index of sample that just before the current position.
		size_t idx2;	//!< Index of sample that just after the current position.
		float ratio;	//!< The ratio between idx1 and idx2 that define the current position.

		template<class T> T& cast() { return *reinterpret_cast<T*>(v); }
		template<class T> const T& cast() const { return *reinterpret_cast<const T*>(v); }
	};	// Interpolation

	typedef FixStrideArray<Interpolation> Interpolations;

// Operations
	/*!	Can be invoked multiple times.
		\param trackSampleCount Array specifing there are how many sample for each track.
	 */
	sal_checkreturn bool init(const StrideArray<const size_t>& trackSampleCount);

	//!	Get interpolation results at a specific position.
	virtual void interpolate(float pos, const Pose& result) const;

	void interpolateSingleTrack(float trackPos, float totalLen, TrackValue& result, size_t trackIndex) const;

	/*!	Check that the data has no problem (eg frame position not in ascending order).
		\return False if something wrong.
	 */
	sal_checkreturn bool checkValid() const;

	/*!	Swap with another AnimationClip, used in AnimationClipLoader to minimize memory copy.
		\note The resource name will not be swap.
	 */
	void swap(AnimationClip& rhs);

// Attributes
	//!	Number of track. For example, one track for position, another track for color.
	size_t trackCount() const;

	//! The length of the specific track.
	float lengthForTrack(size_t trackIndex) const;

	/*!	Get the samples for the track at \em index.
		What it does actually is just return a slice of \em samples.
	 */
	Samples getSamplesForTrack(size_t trackIndex);

	/*!	The samples for all tracks, packed in a single array.
		To get the key frames of a specific track, use getSamplesForTrack().
		The memory layout is chaning sample at a faster rate:
		t1s1, t1s2, t1s3, t2s1, t2s2, t3s1, t3s2, ...
	 */
	Samples samples;

	/*!	The flags for each track (e.g. interpolation methods)
		The array size should be trackCount().
		By default the values are initialized to Flags::Linear.
	 */
	Tracks tracks;

	//! Explicit length of the whole clip (in unit of sample position).
	float length;

	/*! Framerate for this animation.
		A value of 30 means there are 30 frames (31 samples) per second.
	 */
	float framerate;

	//! The loop count of this clip, 0 for infinite loop.
	size_t loopCount;

protected:
	sal_override ~AnimationClip();
};	// AnimationClip

typedef IntrusivePtr<AnimationClip> AnimationClipPtr;

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONTRACK__
