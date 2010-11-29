#include "Pch.h"
#include "TransformAnimator.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Quaternion.h"

namespace MCD {

TransformAnimator::TransformAnimator()
	: trackOffset(0), trackPerEntity(0)
{
}

void TransformAnimator::update()
{
	if(!animation || affectingEntities.empty())
		return;

	const AnimationState::Pose& pose = animation->getPose();
	MCD_ASSUME(trackPerEntity > 0);

	// Update the Entity's transform from the animated pose.
	for(size_t i=0; i<affectingEntities.size(); ++i)
	{
		MCD_ASSERT(affectingEntities[i]);
		Mat44f& mat = affectingEntities[i]->localTransform;
		const size_t i2 = trackOffset + i * trackPerEntity;

		// Position
		if(trackPerEntity > 0)
			mat.setTranslation(pose[i2 + 0].cast<Vec3f>());	// The fourth component is ignored

		// Rotation
		if(trackPerEntity > 1) {
			Mat33f m;
			pose[i2 + 1].cast<Quaternionf>().toMatrix(m);
			MCD_ASSERT("Not pure rotation matrix!" && Mathf::isNearEqual(m.determinant(), 1, 1e-5f));
			mat.setMat33(m);
		}

		// Scale
		// Since the scale is always be one after appling the unit quaternion,
		// therefore using scaleBy() is faster than using setScale().
		if(trackPerEntity > 2)
			mat.scaleBy(pose[i2 + 2].cast<Vec3f>());	// The fourth component is ignored
	}
}

}	// namespace MCD
