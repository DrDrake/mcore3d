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
	AnimationInstance();

	~AnimationInstance();

	AnimationInstance(const AnimationInstance& rhs);

	AnimationInstance& operator=(const AnimationInstance& rhs);

// Operations
	void update();

	/*!	May fail if the sub-track count are not matched.
		\parameter framerate Override AnimationTrack::naturalFramerate if it's value is larger than zero.
	 */
	sal_checkreturn bool addTrack(AnimationTrack& track, float weight=1.0f, float framerate=0.0f);

	//! Will perform normalizeWeight() automatically.
	void removeTrack(size_t index);

	//! Will ignore null track.
	void normalizeWeight();

// Attributes
	struct WeightedTrack
	{
		float weight;
		float frameRate;
		AnimationTrackPtr track;
	};	// WeightedTrack

	size_t trackCount() const;

	size_t subtrackCount() const;

	/*!	Return null if index out of range.
		\note Remember to call normalizeWeight() after you have changed WeightedTrack::weight.
	 */
	sal_maybenull WeightedTrack* getTrack(size_t index);

	sal_maybenull const WeightedTrack* getTrack(size_t index) const;

	//! Indicating all tracks data are committed, ie. ready to call update().
	sal_checkreturn bool isAllTrackCommited() const;

	//! Ok, this simple variable need not to be absolutely thread-safe.
	float time;

	// TODO: Store AnimationTrack::frame2Idx as an optimization.
	const AnimationTrack::KeyFrames interpolatedResult;

protected:
	//! The cache need to recreate when new track is added.
	sal_checkreturn bool resetInterpolatedResult();

	typedef std::vector<WeightedTrack> Tracks;
	Tracks mTracks;

	mutable RecursiveMutex mMutex;
};	// AnimationInstance

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONINSTANCE__
