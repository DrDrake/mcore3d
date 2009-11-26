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

	Restriction: all sub-track are having the same number of key frame.

	Example:
	\code
	AnimationTrackPtr track = new AnimationTrack(L"trackName");

	{	track->acquireWriteLock();
		track->init(2, 1);	// One sub-track with 2 frame

		// Fill the frame time
		track->keyframeTimes[0] = 0.0f;
		track->keyframeTimes[1] = 1.0f;

		// Fill the attribute
		AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
		reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
		reinterpret_cast<Vec4f&>(frames[1]) = Vec4f(2);

		track->releaseWriteLock();
	}

	// For each frame:
	track->acquireReadLock();
	track->updateNoLock(currentAnimationTime);

	// Get the interpolated attributes and do what ever you need.
	const Vec4f& pos = reinterpret_cast<const Vec4f&>(track->interpolatedResult[0]);
	track->releaseReadLock();
	\endcode
 */
class MCD_CORE_API AnimationTrack : public Resource
{
public:
	enum Flags
	{
		Linear	= 1,
		Slerp	= 2,
	};	// Flags

	//! We use a float 4 array to act as a generic datatype for an animated attribute.
	struct KeyFrame
	{
		float v[4];
	};	// KeyFrame

	typedef FixStrideArray<KeyFrame> KeyFrames;

	explicit AnimationTrack(const Path& fileId);

// Operations
	//!	Can be invoked multiple times.
	sal_checkreturn bool init(size_t keyFrameCnt, size_t subtrackCnt);

	/*!	Set the track's time to a specific value, and cache the interpolated result,
		this result can be retrived using \em interpolatedResult.
	 */
	void update(float currentTime);

	//!	 The no lock version of update().
	void updateNoLock(float currentTime);

	/*!	Check that the data has no problem (eg key time not in ascending order).
		\return False if something wrong.
	 */
	sal_checkreturn bool checkValid() const;

	void acquireReadLock();

	void releaseReadLock();

	/*!	Since there will be a seperated thread for animation update, me must
		try to acquire a mutex lock before we can modify the data in AnimationTrack.
		After the write lock is acquired, isCommitted() will return false.
	 */
	void acquireWriteLock();

	/*!	The paired function with acquireLock().
		After the write lock is released, isCommitted() will return true.
	 */
	void releaseWriteLock();

	/*!	To indicate that all animation data are ready to use.
		Returns true after the wirte lock is released.
		Returns false if the write lock is never acquired.
	 */
	sal_checkreturn bool isCommitted() const;

// Attributes
	/*!	Number of keyframes for each sub-track.
		All sub-track are having the same number of key frame.
	 */
	size_t keyframeCount() const { return keyframeTimes.size; }

	/*!	Number of sub-track.
		For example, one sub-track for position, another sub-track for color.
	 */
	size_t subtrackCount() const { return subtrackFlags.size; }

	//! The total time of the animation, ie. keyframeTimes[keyframeCount() - 1].
	float totalTime() const;

	float currentTime() const;

	/*!	Get the key frames for the sub-track at \em index.
		What it does actually is just return a slice of \em keyframes.
	 */
	KeyFrames getKeyFramesForSubtrack(size_t index);

	/*!	The keyframes for each sub tracks, packed in a single array.
		The array size should be keyframeCount() * subtrackCount().
		To get the key frames of a specific sub-track, use getKeyFramesForSubtrack().
	 */
	KeyFrames keyframes;

	/*!	Sorted (in ascending order) keyframe time shared by each sub-track.
		The array size should be keyframeCount().
	 */
	FixStrideArray<float> keyframeTimes;

	/*!	The flags for each subtrack (e.g. interpolation methods)
		The array size should be subtrackCount().
		By default the values are initialized to Flags::Linear.
	 */
	FixStrideArray<Flags> subtrackFlags;

	//! Whether the current time will wrap over if it's larger than totalTime().
	bool loop;

	// Cached variables, updated after calling update()
	size_t frame1Idx;	//!< Index to keyframeTimes that just before the current time.
	size_t frame2Idx;	//!< Index to keyframeTimes that just after the current time.
	float ratio;		//!< The ratio between frame1Idx and frame2Idx that define the current time.
	KeyFrames interpolatedResult;

protected:
	sal_override ~AnimationTrack();

	/*!	A boolean variable to indicate all the data are ready, meaning animation thread
		can safely READ the data.
		If modifing the animation data is needed, set this flag to false first.
	 */
	bool mCommitted;

	Mutex mMutex;
};	// AnimationTrack

typedef IntrusivePtr<AnimationTrack> AnimationTrackPtr;

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONTRACK__
