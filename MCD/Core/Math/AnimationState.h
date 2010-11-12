#ifndef __MCD_CORE_MATH_ANIMATIONSTATE__
#define __MCD_CORE_MATH_ANIMATIONSTATE__

#include "AnimationClip.h"
#include "../System/StringHash.h"
#include <vector>

namespace MCD {

/// Using a set of variables to define a specific pose in an AnimationClip
class MCD_CORE_API AnimationState
{
public:
	AnimationState();

	~AnimationState();

	AnimationState(const AnimationState& rhs);

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

	/// To speed up the serach of key data.
	KeyIdxHint keyIdxHint;

// Operations
	void assignTo(const Pose& pose);

	/// Additive blend the calculated animation pose to the accumulating pose.
	/// Return the updated accumulatedWeight.
	float blendResultTo(const Pose& accumulatePose, float accumulatedWeight);

protected:
	void allocateIdxHint();
};	// AnimationState

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONSTATE__
