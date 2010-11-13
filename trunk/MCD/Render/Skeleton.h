#ifndef __MCD_RENDER_SKELETON__
#define __MCD_RENDER_SKELETON__

#include "Animation.h"
#include "../Core/Math/Mat44.h"

namespace MCD {

typedef IntrusiveWeakPtr<class Entity> EntityPtr;

// TODO: Utilize StrideArray to unify the scattered std::vectors.
class MCD_RENDER_API Skeleton : public Resource
{
public:
	explicit Skeleton(const Path& fileId);

// Operations
	/// Resize the various array, including \em basePose.
	void init(size_t jointCount);

	/// Calculate the inverse of basePose's matrix and assign to \em basePoseInverse.
	void initBasePoseInverse();

	/// Returns -1 if the name cannot be found.
	int findJointByName(const char* name) const;

	/// Swap with another Skeleton, used in SkeletonLoader to minimize memory copy.
	/// @note The resource name will not be swap.
	void swap(Skeleton& rhs);

// Attributes
	/// Array of index specifing the parent of a joint.
	/// The root joint will indicate by an index which point to itself, ie at index = 0.
	std::vector<size_t> parents;

	///	Name of each joint
	std::vector<std::string> names;

	///	Transforms in the basePose are relative to the root joint.
	std::vector<Mat44f> basePose;

	/// Usefull for skinning using the base pose, or getting back the joint's location,
	/// even the base pose is already baked into the animation.
	std::vector<Mat44f> basePoseInverse;

protected:
	sal_override ~Skeleton();
};	// Skeleton

typedef IntrusivePtr<Skeleton> SkeletonPtr;

/// Skeleton, consist of a number of animated joints which are represented by a transformation matrix.
///	@sa http://cache-www.intel.com/cd/00/00/29/37/293749_293749.pdf
/// @todo http://www.unrealtechnology.com/features.php?ref=animation
class MCD_RENDER_API SkeletonPose : public AnimatedComponent
{
public:
	SkeletonPose();

	sal_override const std::type_info& familyType() const {
		return typeid(SkeletonPose);
	}

// Cloning
	sal_override sal_checkreturn bool cloneable() const;

	sal_override sal_maybenull Component* clone() const;

	sal_checkreturn bool postClone(const Entity& src, Entity& dest);

// Operations
	/// Replicate the bone structure as a tree of Entities, usefull for attachement purpose.
	void createBoneEntity();

// Attributes
	/// Stores all the transforms of the joints, in mesh space.
	/// The hierarchical nature of the skeleton is flattened using breadth-first traversal.
	/// When updating the transforms, be sure to update them using a front to back order,
	/// such that the parents are updated before the children.
	std::vector<Mat44f> transforms;

	SkeletonPtr skeleton;
	
	size_t subTrackOffset;	//!< Starting index to the animation's sub-tracks
	AnimationComponentPtr animation;
	static const size_t trackPerJoint = 2;

	/// If not emtpy, the animated bone will also apply to these Entity's transform
	std::vector<EntityPtr> boneEntities;

protected:
	sal_override void update();
};	// SkeletonPose

typedef IntrusiveWeakPtr<SkeletonPose> SkeletonPosePtr;

}	// namespace MCD

#endif	// __MCD_RENDER_SKELETON__
