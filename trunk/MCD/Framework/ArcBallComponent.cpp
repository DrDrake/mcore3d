#include "Pch.h"
#include "ArcBallComponent.h"
#include "../Core/Entity/InputComponent.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Mat44.h"

namespace MCD {

ArcBallComponent::ArcBallComponent()
{
	radius = 20;
	minRadius = 2;
	rotationSpeed = 0.5f * 3.14159265f / 180;
	translationSpeed = 0.2f;
	rotation = Vec2f::cZero;
}

void ArcBallComponent::update(float dt)
{
	if(!inputComponent || !target)
		return;

	// Make a short-cut first
	Mat44f& localTransform = entity()->localTransform;

	Vec2f currentMouseAxis = Vec2f(inputComponent->getAxis("mouse x"), inputComponent->getAxis("mouse y"));
	Vec2f deltaRotation(0);
	Vec2f deltaTranslation(0);

	// Handle radius (mouse wheel)
	radius -= inputComponent->getAxisDelta("mouse z") * translationSpeed;

	// Handling rotation (left drag)
	if(inputComponent->getMouseButton(0))
		deltaRotation = currentMouseAxis - mLastMouseAxis;

	if(inputComponent->getMouseButton(1))
		deltaTranslation = currentMouseAxis - mLastMouseAxis;

	// Perform the target's transform
	deltaRotation *= -rotationSpeed;
	rotation += deltaRotation;

	Mat44f horizontalRotation = Mat44f::makeAxisRotation(Vec3f(0, 1, 0), rotation.x);
	Mat44f verticalRotation = Mat44f::makeAxisRotation(horizontalRotation.xBiasVector(), rotation.y);
	Mat44f rotation = verticalRotation * horizontalRotation;

	Vec3f trans = translationSpeed * Vec3f(deltaTranslation.x, 0, deltaTranslation.y);
	rotation.transformNormal(trans);
	localTransform.translateBy(trans);

	Vec3f camPos(0, 0, radius);
	rotation.transformPoint(camPos);
	target->localTransform = rotation;
	target->localTransform.setTranslation(camPos + localTransform.translation());

	mLastMouseAxis = currentMouseAxis;
}

}	// namespace MCD
