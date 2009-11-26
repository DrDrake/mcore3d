#include "Pch.h"
#include "AnimationComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/Math/AnimationInstance.h"
#include "../../Core/Math/Quaternion.h"

namespace MCD {

AnimationComponent::AnimationComponent()
	: animationInstance(*new AnimationInstance)
	, animationInstanceHolder(&animationInstance)
{
}

AnimationComponent::~AnimationComponent()
{
}

void AnimationComponent::update()
{
	if(!entity())
		return;

	animationInstance.time += float(timer.getDelta().asSecond());

	// TODO: Put this in multi-thread.
	animationInstance.update();

	// Update the Entity's transform from the interpolatedResult.
	const AnimationTrack::KeyFrames& frames = animationInstance.interpolatedResult;

	Mat44f& mat = entity()->localTransform;

	// Position
	if(frames.size > 0)
		mat.setTranslation(reinterpret_cast<const Vec3f&>(frames[0]));	// The fourth component is ignored

	if(frames.size > 1) {
		Mat33f m;
		reinterpret_cast<const Quaternionf&>(frames[1]).toMatrix(m);
		MCD_ASSERT(Mathf::isNearEqual(m.determinant(), 1, 1e-5f));
		mat.setMat33(m);
	}

	// Since the scale is always be one after appling the unit quaternion,
	// therefore using scaleBy() is faster than using setScale().
	if(frames.size > 2)
		mat.scaleBy(reinterpret_cast<const Vec3f&>(frames[2]));	// The fourth component is ignored

	// TODO: Update the color.
}

}	// namespace MCD
