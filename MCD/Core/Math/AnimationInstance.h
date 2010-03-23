#ifndef __MCD_CORE_MATH_ANIMATIONINSTANCE__
#define __MCD_CORE_MATH_ANIMATIONINSTANCE__

#include "AnimationTrack.h"
#include <vector>

namespace MCD {

/*!	Stores per-instance data where the sharing semantic AnimationTrack didn't has.

	This class is thread-safe in a way that update() and isAllTrackCommited()
	can run in thread A while all other functions run in thread B.
 */
class MCD_CORE_API AnimationInstance
{
public:
	struct KeyFrame
	{
		float v[4];
	};	// KeyFrame

	struct WeightedTrack
	{
		float weight;
		float frameRate;
		std::string name;
		AnimationTrackPtr track;
	};	// WeightedTrack

	typedef FixStrideArray<KeyFrame> KeyFrames;
	typedef AnimationTrack::Interpolation Interpolation;
	typedef AnimationTrack::Interpolations Interpolations;

	AnimationInstance();

	~AnimationInstance();

	AnimationInstance(const AnimationInstance& rhs);

	AnimationInstance& operator=(const AnimationInstance& rhs);

// Operations
	//!	Calculat the interpolation results according to the member variable \em time.
	void update();

	/*!	May fail if the sub-track count are not matched.
		\parameter framerate Override AnimationTrack::naturalFramerate if it's value is larger than zero.
	 */
	sal_checkreturn bool addTrack(AnimationTrack& track, float weight=1.0f, float framerate=0.0f, sal_in_z const char* name="unnamed");

	//! Will perform normalizeWeight() automatically.
	void removeTrack(size_t index);

	//! Will ignore null track.
	void normalizeWeight();

// Attributes
	size_t trackCount() const;

	size_t subtrackCount() const;

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
