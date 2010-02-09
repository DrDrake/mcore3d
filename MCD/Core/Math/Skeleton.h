#ifndef __MCD_CORE_MATH_SKELETON__
#define __MCD_CORE_MATH_SKELETON__

#include "Mat44.h"
#include "AnimationInstance.h"

namespace MCD {

/*!	Skeleton, consist of a number of joints which are represented by a transformation matrix.
	\sa http://cache-www.intel.com/cd/00/00/29/37/293749_293749.pdf
	\todo http://www.unrealtechnology.com/features.php?ref=animation

	\note Applying the skeleton is implemented in the Render module,
		since there may have software or hardware method.
 */
class MCD_CORE_API Skeleton
{
public:
// Operations
	//!	Resize the various array.
	void init(size_t jointCount);

// Attributes
	/*!	Stores all the transforms of the joints, in mesh space.
		The hierarchical nature of the skeleton is flattened using breadth-first traversal.
		When updating the transforms, be sure to update them using a front to back order,
		such that the parents are updated before the children.
	 */
	std::vector<Mat44f> transforms;

	/*!	Array of index specifing the parent of a joint.
		The root joint will indicate by an index which point to itself, ie at index = 0.
	 */
	std::vector<size_t> parents;

	//!	Usefull for getting back the joint's location, even the base pose is already baked into the animation.
	std::vector<Mat44f> basePoseInverse;

	//! Including the root joint.
	size_t jointCount() const {
		return transforms.size();
	}

	Mat44f& rootJointTransform() { return transforms[0]; }
	const Mat44f& rootJointTransform() const { return transforms[0]; }
};	// Skeleton

class MCD_CORE_API SkeletonAnimation
{
public:
	enum TrackIdx {
		Translation = 0,
		Rotation = 1
	};

	/*!	Transforms local joint matrices (relative to parent joints) to global joint matrices (object or world space).

		The routine works on skeleton::transforms and transforms the joint matrices in-place.
		Furthermore the index of the first and last joint of a sequence of joints that need to be
		transformed are specified. This allows the complete skeleton to be split up in multiple
		sequences of joints that are transformed. Joints inbetween such sequences can then be
		transformed separately and additional modifications can be applied where necessary.

		The root joints' transform should be assigned before every call of this function.

		Assertion fail if the skeleton's size didn't match the track, or firstJoint/lastJoint is out of range.
	 */
	void applyTo(Skeleton& skeleton, int firstJoint=-1, int lastJoint=-1);

	/*!	The track suppose to have sub-track(s) of joint's rotation, translation and or scale.
		Assumptions:
			The sub-track index corresponds to Skeleton::transforms.
			The order of the sub-tracks are assumed to be rotation, translation and then scale.
	 */
	AnimationInstance anim;
};	// SkeletonAnimation

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SKELETON__
