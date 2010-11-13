#include "Pch.h"
#include "Skeleton.h"
#include "Animation.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/System/Log.h"

namespace MCD {

Skeleton::Skeleton(const Path& fileId)
	: Resource(fileId)
{}

void Skeleton::init(size_t jointCount)
{
	parents.resize(jointCount);
	names.resize(jointCount);
	basePose.resize(jointCount);
	basePoseInverse.resize(jointCount);
}

void Skeleton::initBasePoseInverse()
{
	bool ok = true;
	for(size_t i=0; i<basePoseInverse.size(); ++i)
		ok = ok && basePose[i].inverse(basePoseInverse[i]);
	if(!ok)
		Log::write(Log::Warn, "Skeleton::initBasePoseInverse() failed");
}

int Skeleton::findJointByName(const char* name) const
{
	for(size_t i=0; i<names.size(); ++i)
		if(names[i] == name )
			return int(i);
	return -1;
}

void Skeleton::swap(Skeleton& rhs)
{
	std::swap(parents, rhs.parents);
	std::swap(names, rhs.names);
	std::swap(basePose, rhs.basePose);
	std::swap(basePoseInverse, rhs.basePoseInverse);
}

Skeleton::~Skeleton()
{}

SkeletonPose::SkeletonPose()
	: subTrackOffset(0)
{
}

bool SkeletonPose::cloneable() const {
	return true;
}

Component* SkeletonPose::clone() const
{
	SkeletonPose* cloned = new SkeletonPose;

	cloned->transforms = this->transforms;
	cloned->skeleton = this->skeleton;

	return cloned;
}

bool SkeletonPose::postClone(const Entity& src, Entity& dest)
{
	// Find the Component in the src tree that corresponding to this
	SkeletonPose* srcComponent = dynamic_cast<SkeletonPose*>(
		ComponentPreorderIterator::componentByOffset(src, ComponentPreorderIterator::offsetFrom(dest, *this))
	);

	if(!srcComponent)
		return false;
	if(!srcComponent->animation)
		return false;

	// Find the Component in the src tree that corresponding to srcComponent->animation
	animation = dynamic_cast<AnimationComponent*>(
		ComponentPreorderIterator::componentByOffset(dest, ComponentPreorderIterator::offsetFrom(src, *srcComponent->animation))
	);

	boneEntities.clear();
	for(size_t i=0; i<srcComponent->boneEntities.size(); ++i) {
		if(EntityPtr targetSrc = srcComponent->boneEntities[i]) {
			// Find the Component in the src tree that corresponding to referenceToAnother
			EntityPtr targetDest = EntityPreorderIterator::entityByOffset(dest, EntityPreorderIterator::offsetFrom(src, *targetSrc));
			if(targetDest)
				boneEntities.push_back(targetDest);
		}
		else
			boneEntities.push_back(nullptr);
	}

	return true;
}

enum TrackIdx {
	Translation = 0,
	Rotation = 1
};

void SkeletonPose::update()
{
	if(!animation || !skeleton) return;

	const size_t jointCount = skeleton->parents.size();
	if(transforms.size() < jointCount)
		transforms.resize(jointCount, Mat44f::cIdentity);

	transforms[0] = Mat44f::cIdentity;	// Set the root transform

	Mat44f m = Mat44f::cIdentity;
	Mat33f tmp;

	const AnimationState::Pose& pose = animation->getPose();
	MCD_ASSERT(pose.data);

	for(size_t i=0; i<jointCount; ++i) {
		const size_t i2 = subTrackOffset + i * trackPerJoint;
		pose[i2 + Rotation].cast<Quaternionf>().toMatrix(tmp);
		MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(tmp.determinant(), 1, 1e-5f));

		m.setMat33(tmp);
		m.setTranslation(pose[i2 + Translation].cast<Vec3f>());

		if(i < boneEntities.size())
			boneEntities[i]->localTransform = m;

		transforms[i] = transforms[skeleton->parents[i]] * m;
	}
}

void SkeletonPose::createBoneEntity()
{
	// Clear all existing boneEntities first
	for(size_t i=0; i<boneEntities.size(); ++i)
		boneEntities[i]->destroyThis();
	boneEntities.clear();

	const size_t jointCount = skeleton->parents.size();

	boneEntities.resize(jointCount);
	for(size_t i=0; i<jointCount; ++i)
		boneEntities[i] = new Entity(skeleton->names[i].c_str());

	// NOTE: Traverse in reverse order
	for(size_t i=jointCount; --i; ) {
		const size_t parentIdx = skeleton->parents[i];
		boneEntities[i]->asChildOf(boneEntities[parentIdx]);
	}

	entity()->addLastChild(boneEntities[0]);
}

}	// namespace MCD
