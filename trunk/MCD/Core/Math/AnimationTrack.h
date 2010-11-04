#ifndef __MCD_CORE_MATH_ANIMATIONTRACK__
#define __MCD_CORE_MATH_ANIMATIONTRACK__

#include "../System/Array.h"
#include "../System/Resource.h"

namespace MCD {

/*!	A kind of Resource which stores animation data.
	In most cases user should use AnimationInstance for working with
	per instance information like current animation time, weight etc.

	For thread safty reasons, all data read and write operations should be enclosed
	in read lock and write lock respectively.

	Restriction: all sub-track should have the same number of key frame.

	\note Track pos / frame rate = time

	\note The memory layout of the key frames is:
	subTrack1Frame1, subTrack2Frame1, subTrack1Frame2, subTrack2Frame2, ...

	Example:
	\code
	AnimationClipPtr track = new AnimationClip(L"trackName");

	{	AnimationClip::ScopedWriteLock lock(*track);
		track->init(2, 1);	// One sub-track with 2 frame
		AnimationClip::KeyFrames frames = track->getKeyFramesForSubtrack(0);

		// Fill the frame's position
		frames[0].pos = 0;
		frames[1].pos = 1;

		// Fill the attribute
		reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);
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
	};	// Flags

	//! We use a float 4 array to act as a generic datatype for an animated attribute.
	struct KeyFrame
	{
		float v[4];
		float pos;	//!< Indicate the position of this frame belongs in the track.
	};	// KeyFrame

	typedef FixStrideArray<KeyFrame> KeyFrames;

	struct Subtrack
	{
		const size_t index;	//!< Index to the \em keyframes member variable. Assigned by init(), don't modify it.
		size_t frameCount;	//!< Number of frames for a sub-track
		Flags flag;
	};	// Subtrack

	typedef FixStrideArray<Subtrack> Subtracks;

	struct Interpolation
	{
		MCD_CORE_API Interpolation();
		float v[4];
		size_t frame1Idx;	//!< Index of key frame that just before the current position.
		size_t frame2Idx;	//!< Index of key frame that just after the current position.
		float ratio;		//!< The ratio between frame1Idx and frame2Idx that define the current position.
	};	// Interpolation

	typedef FixStrideArray<Interpolation> Interpolations;

	struct MCD_CORE_API ScopedReadLock {
		ScopedReadLock(const AnimationClip& a);
		~ScopedReadLock();
		const AnimationClip& track;
	};	// ScopedReadLock

	struct MCD_CORE_API ScopedWriteLock {
		ScopedWriteLock(const AnimationClip& a);
		~ScopedWriteLock();
		const AnimationClip& track;
	};	// ScopedWriteLock

// Operations
	/*!	Can be invoked multiple times.
		\param subtrackFrameCount Array specifing there are how many key frames for each sub-track.
	 */
	sal_checkreturn bool init(const StrideArray<const size_t>& subtrackFrameCount);

	/*!	Get interpolation results at a specific track position.
		Set the variable \em loopOverride with 0 to force loop, 1 for no loop and -1 for using AnimationClip::loop.
		\return The wrapped or clamped \em trackPos for out of bound condition.
		\note With acquireWriteLock() and releaseWriteLock() implied.
	 */
	float interpolate(float trackPos, const Interpolations& result, int loopOverride=-1) const;

	//!	 The no lock version of interpolate().
	float interpolateNoLock(float trackPos, const Interpolations& result, int loopOverride=-1) const;

	/*!	Check that the data has no problem (eg frame position not in ascending order).
		\return False if something wrong.
	 */
	sal_checkreturn bool checkValid() const;

	void acquireReadLock() const;

	void releaseReadLock() const;

	/*!	Since there will be a seperated thread for animation update, me must
		try to acquire a mutex lock before we can modify the data in AnimationClip.
		After the write lock is acquired, isCommitted() will return false.
	 */
	void acquireWriteLock() const;

	/*!	The paired function with acquireLock().
		After the write lock is released, isCommitted() will return true.
	 */
	void releaseWriteLock() const;

	/*!	Swap with another AnimationClip, used in AnimationClipLoader to minimize memory copy.
		\note The resource name will not be swap.
	 */
	void swap(AnimationClip& rhs);

// Attributes
	/*!	Number of sub-track.
		For example, one sub-track for position, another sub-track for color.
	 */
	size_t subtrackCount() const;

	//!	Number of keyframes for the specific sub-track.
	size_t keyframeCount(size_t subtrackIndex) const;

	//! The length (largest pos) of the specific sub-track.
	float length(size_t subtrackIndex) const;

	//!	The length of the longest sub-track.
	float length() const;

	/*!	Get the key frames for the sub-track at \em index.
		What it does actually is just return a slice of \em keyframes.
	 */
	KeyFrames getKeyFramesForSubtrack(size_t subtrackIndex);

	/*!	The keyframes for each sub tracks, packed in a single array.
		To get the key frames of a specific sub-track, use getKeyFramesForSubtrack().
		The memory layout is:
		t1f1, t1f2, t1f3, t2f1, t2f2, t3f1, t3f2, ...
	 */
	KeyFrames keyframes;

	/*!	The flags for each subtrack (e.g. interpolation methods)
		The array size should be subtrackCount().
		By default the values are initialized to Flags::Linear.
	 */
	Subtracks subtracks;

	//! Whether the current track position will wrap over if it's larger than length().
	bool loop;

	/*! The natural framerate for this animation (default value is 1).
		This variable is not used directly in this class, but to be used by client
		in order to pass the correct trackPos value to update() function.
	 */
	float naturalFramerate;

	/*!	To indicate that all animation data are ready to use.
		Returns true after the wirte lock is released.
		Returns false if the write lock is never acquired.
	 */
	sal_checkreturn bool isCommitted() const;

protected:
	sal_override ~AnimationClip();

	void interpolateSingleSubtrack(float trackPos, Interpolation& result, size_t trackIndex, int loopOverride) const;

	//!	The length of the longest sub-track, assigned in releaseWriteLock().
	mutable float mLength;

	/*!	A boolean variable to indicate all the data are ready, meaning animation thread
		can safely READ the data.
		If modifing the animation data is needed, set this flag to false first.
	 */
	mutable bool mCommitted;

	//!	Protect between the reader (update function) and the writer (animation loader).
	mutable Mutex mMutex;
};	// AnimationClip

typedef IntrusivePtr<AnimationClip> AnimationClipPtr;

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONTRACK__
