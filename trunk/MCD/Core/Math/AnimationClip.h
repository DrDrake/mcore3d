#ifndef __MCD_CORE_MATH_ANIMATIONTRACK__
#define __MCD_CORE_MATH_ANIMATIONTRACK__

#include "Vec4.h"
#include "../System/Array.h"
#include "../System/Resource.h"

namespace MCD {

/*!	A kind of Resource which stores animation data.

	The atomic key data type is float4, which may be interpreted as a point, vector,
	quaternion or color in the higher level parts of the animation system.

	\note The memory layout of the samples is:
	track1Sample1, track1Sample2, track2Sample1, track2Sample2, ...

	Example:
	\code
	AnimationClipPtr clip = new AnimationClip(L"clipName");

	{	clip->init(2, 1);	// One track with 2 key
		AnimationClip::Samples samples = track->getSamplesForTrack(0);

		// Fill the key's position
		keys[0].pos = 0;
		keys[1].pos = 1;

		// Fill the attribute
		reinterpret_cast<Vec4f&>(samples[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(samples[1]) = Vec4f(2);
	}

	// For each frame:
	track->updateNoLock(currentTrackPos);

	// Get the interpolated attributes and do what ever you need.
	const Vec4f& pos = reinterpret_cast<const Vec4f&>(track->interpolatedResult[0]);
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

	/// We use a float 4 array to act as a generic datatype for an animated attribute.
	struct Key
	{
		Vec4f v;
		float pos;			///< The position where the key is taken (float is used instead of integer to reduce type casting, which may be expensive)

		template<class T> T& cast() { return reinterpret_cast<T&>(v); }
		template<class T> const T& cast() const { return reinterpret_cast<const T&>(v); }
	};	// Key

	typedef FixStrideArray<Key> Keys;

	/// Provide description on how keyBuffer is interpreted.
	struct Track
	{
		const size_t index;		///< Index to the \em keyBuffer member variable. Assigned by init(), don't modify it.
		const size_t keyCount;	///< Number of key for this track.
		Flags flag;
	};	// Track

	typedef FixStrideArray<Track> Tracks;

	/// Represent a single sample
	struct Sample
	{
		Vec4f v;
		Flags flag;

		void blend(float t, const Sample& s1, const Sample& s2);

		template<class T> T& cast() { return reinterpret_cast<T&>(v); }
		template<class T> const T& cast() const { return reinterpret_cast<const T&>(v); }
	};	// Sample

	typedef FixStrideArray<Sample> Pose;

	typedef FixStrideArray<uint16_t> KeyIdxHint;

// Operations
	/// Reserve memory for key storage, suitable for the case where the number of
	/// tracks and keys are all know in advance.
	/// @note Can be invoked multiple times.
	/// @param trackKeyCount Array specifing there are how many key for each track.
	sal_checkreturn bool init(const StrideArray<size_t>& trackKeyCount);

	/// Create the key storage track by track, suitable for the case where the number of
	/// tracks and keys are NOT know in advance.
	void addTrack(size_t keyCount, Flags flag);

	///	Get interpolation results at a specific position.
	virtual void sample(float pos, const Pose& result, const KeyIdxHint& hint=KeyIdxHint(nullptr,0)) const;

	/// Returns the new keySearchHint
	size_t sampleSingleTrack(float trackPos, float totalLen, Sample& result, size_t trackIndex, size_t keySearchHint=0) const;

	///	Check that the data has no problem (eg frame position not in ascending order).
	/// \return False if something wrong.
	sal_checkreturn bool checkValid() const;

	///	Swap with another AnimationClip, used in AnimationClipLoader to minimize memory copy.
	/// \note The resource name will not be swap.
	void swap(AnimationClip& rhs);

	/// Create a clip that represent the difference between the master clip and the target clip.
	/// The master and target clip should have the same number of tracks and their flags should be the same too.
	sal_checkreturn bool createDifferenceClip(AnimationClip& master, AnimationClip& target);

// Attributes
	///	Number of track. For example, one track for position, another track for color.
	size_t trackCount() const;

	/// The length of the specific track.
	float lengthForTrack(size_t trackIndex) const;

	/// Get the keys for the track at \em index.
	/// What it does actually is just return a slice of \em samples.
	Keys getKeysForTrack(size_t trackIndex);

	/// The keys for all tracks, packed in a single array.
	/// To get the key frames of a specific track, use getKeysForTrack().
	/// The memory layout is chaning key at a faster rate:
	/// t1s1, t1s2, t1s3, t2s1, t2s2, t3s1, t3s2, ...
	Keys keyBuffer;

	/// The flags for each track (e.g. interpolation methods)
	/// The array size should be trackCount().
	/// By default the values are initialized to Flags::Linear.
	Tracks tracks;

	/// Explicit length of the whole clip (in unit of key position).
	float length;

	/// Framerate for this animation.
	/// A value of 30 means there are 30 frames (31 keys) per second.
	float framerate;

	/// The loop count of this clip, 0 for infinite loop.
	size_t loopCount;

protected:
	sal_override ~AnimationClip();
};	// AnimationClip

typedef IntrusivePtr<AnimationClip> AnimationClipPtr;

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONTRACK__
