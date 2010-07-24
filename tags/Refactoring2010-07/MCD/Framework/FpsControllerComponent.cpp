#include "Pch.h"
#include "FpsControllerComponent.h"
#include "../Core/Entity/InputComponent.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Math/Mat44.h"

namespace MCD {

FpsControllerComponent::FpsControllerComponent()
{
	translationSpeed = 10;
	mouseSensitivity = 0.5f * 3.14159265f / 180;
	mAccumulateMouseDelta = Vec2f::cZero;
}

void FpsControllerComponent::update(float dt)
{
	if(!inputComponent || !target)
		return;

	// Make a short-cut first
	Mat44f localTransform = target->localTransform;

	{	// Handling rotation
		Vec2f currentMouseAxis = Vec2f(inputComponent->getAxis("mouse x"), inputComponent->getAxis("mouse y"));

		if(inputComponent->getMouseButton(0))
		{
			Vec2f delta = currentMouseAxis - mLastMouseAxis;
			delta *= -mouseSensitivity;
			mAccumulateMouseDelta += delta;

			Mat44f horizontalRotation = Mat44f::makeAxisRotation(Vec3f(0, 1, 0), mAccumulateMouseDelta.x);
			Mat44f verticalRotation = Mat44f::makeAxisRotation(horizontalRotation.xBiasVector(), mAccumulateMouseDelta.y);

			Vec3f backupTranslation = localTransform.translation();
			localTransform = verticalRotation * horizontalRotation;
			localTransform.translateBy(backupTranslation);
		}
		mLastMouseAxis = currentMouseAxis;
	}

	{	// Handling translation
		Vec3f translation = Vec3f::cZero;

		if(inputComponent->getButton("s"))
			  translation += localTransform.zBiasVector();
		if(inputComponent->getButton("w"))
			  translation -= localTransform.zBiasVector();
		if(inputComponent->getButton("d"))
			  translation += localTransform.xBiasVector();
		if(inputComponent->getButton("a"))
			  translation -= localTransform.xBiasVector();
		if(inputComponent->getButton("PageUp"))
			  translation += localTransform.yBiasVector();
		if(inputComponent->getButton("PageDown"))
			  translation -= localTransform.yBiasVector();

		translation *= dt * translationSpeed;
		localTransform.translateBy(translation);
	}

/*	{	// Handle mouse wheel
		float z = inputComponent->getAxisDelta("mouse z");
		float newFov = fov * (1 - z / 20);
		if(newFov < 140)
			fov = newFov;
	}*/

	target->localTransform = localTransform;
}

}	// namespace MCD
