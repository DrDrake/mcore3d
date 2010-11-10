#ifndef __MCD_CORE_MATH_ANIMATIONINSTANCE__
#define __MCD_CORE_MATH_ANIMATIONINSTANCE__

#include "AnimationClip.h"
#include "../System/StringHash.h"
#include <vector>

namespace MCD {

class MCD_CORE_API AnimationState
{
public:
	AnimationState();

	~AnimationState();

// Attributes
	typedef AnimationClip::Pose Pose;
	typedef AnimationClip::KeyIdxHint KeyIdxHint;

	FixString name;

	float weight;
	float rate;
	int loopCountOverride;	///< Override the loopCount in the clip if > -1.
	int loopCount() const;	///< Get the overall loop count value.

	float worldTime;		///< The current world time. An update function would like to increment this value.
	float worldRefTime;		///< Where this animation start in the world time frame.

	/// The current local time
	/// \note If the animation is still in it's loop, end of local time will became 0.
	float localTime() const;

	/// When the animation will end in the world time frame.
	/// Returns numeric_limits<float>::max() if the animation never end.
	float worldEndTime() const;

	/// Returns true if the animation has reached the end, and is not looping infinitly.
	/// It is the same to perform: worldTime >= worldEndTime()
	bool ended() const;

	AnimationClipPtr clip;

	KeyIdxHint keyIdxHint;

// Operations
	/// Additive blend the calculated animation pose to the accumulating pose.
	void blendResultTo(Pose& accumulatePose, float accumulatedWeight);

protected:
};	// AnimationState

/*!	Stores per-instance data where the sharing semantic AnimationClip didn't has.

	This class is thread-safe in a way that update() and isAllTrackCommited()
	can run in thread A while all other functions run in thread B.
 */
class MCD_CORE_API AnimationInstance
{
public:
// Key frame and weighted track data types
	struct KeyFrame
	{
		float v[4];
	};	// KeyFrame

	struct WeightedTrack
	{
		float weight;
		float frameRate;
		int loopOverride;	//!< To override the AnimationClip::loop variable. Negative means use AnimationClip::loop, 0 for no loop, 1 for loop.
		float lastEventPos;	//!< For tracking which event callback need to invoke since last update(). Internal use, user no need to touch with it.
		std::string name;
		AnimationClipPtr track;
	};	// WeightedTrack

	typedef FixStrideArray<KeyFrame> KeyFrames;
	typedef AnimationClip::Interpolation Interpolation;
	typedef AnimationClip::Interpolations Interpolations;

// Constructors, destructor and assignment operator
	AnimationInstance();

	~AnimationInstance();

	AnimationInstance(const AnimationInstance& rhs);

	AnimationInstance& operator=(const AnimationInstance& rhs);

// Operations
	/*!	Calculat the interpolation results according to the member variable \em time.
		It also invoke the event callback if necessary.
	 */
	void update();

	/*!	May fail if the sub-track count are not matched.
		\parameter framerate Override AnimationClip::naturalFramerate if it's value is larger than zero.
	 */
	sal_checkreturn bool addTrack(AnimationClip& track, float weight=1.0f, float framerate=0.0f, sal_in_z const char* name="unnamed");

	//! Will perform normalizeWeight() automatically.
	void removeTrack(size_t index);

	//! Will ignore null track.
	void normalizeWeight();

// Attributes
	size_t trackCount() const;

	size_t subtrackCount() const;

	//! Returns the length of the longest track.
	float length() const;

	//!	It's the longest track's length() / frameRate.
	float totalTime() const;

	/*!	Return null if index out of range.
		\note Remember to call normalizeWeight() after you have changed WeightedTrack::weight.
	 */
	sal_maybenull WeightedTrack* getTrack(size_t index);
	sal_maybenull const WeightedTrack* getTrack(size_t index) const;

	//!	Return null if the track name is not found.
	sal_maybenull WeightedTrack* getTrack(sal_in_z const char* weightedTrackName);
	sal_maybenull const WeightedTrack* getTrack(sal_in_z const char* weightedTrackName) const;

	//!	Return -1 if the track name is not found.
	sal_checkreturn int getTrackIndex(sal_in_z const char* weightedTrackName) const;

	//! Indicating all tracks data are committed, ie. ready to call update().
	sal_checkreturn bool isAllTrackCommited() const;

	//!	Returns the current frame of the given weighted track, 0 if \em weightedTrackIndex is out of bound.
	sal_checkreturn size_t currentFrame(size_t weightedTrackIndex=0) const;

	/*!	Set the \em time variable by using a particular frame time of a particular weighted track.
		Perform wrapping if loop, otherwise the last frame is used for out of bound frame index.
		Do nothing if \em weightedTrackIndex is out of bound.
	 */
	void setTimeByFrameIndex(size_t frameIndex, size_t weightedTrackIndex=0);

	//! Ok, this simple variable need not to be absolutely thread-safe.
	float time;

	//!	The final weighted interpolation result.
	const KeyFrames weightedResult;

	//!	Store the interpolation results for individual sub-tracks.
	const Interpolations interpolations;

protected:
	//! The cache need to recreate when new track is added.
	sal_checkreturn bool resetInterpolatedResult();

	typedef std::vector<WeightedTrack> Tracks;
	Tracks mTracks;

	mutable RecursiveMutex mMutex;
};	// AnimationInstance

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONINSTANCE__
