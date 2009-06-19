#include "Pch.h"
#include "CameraComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

CameraComponent::CameraComponent()
	: velocity(0), clearColor(0), isMouseDown(false)
{
}

void CameraComponent::render()
{
	Entity* e = entity();
	if(!e)
		return;

	float deltaTime = float(mTimer.getDelta().asSecond());

	if(velocity.length() > 0) {
		camera.moveForward(velocity.z * deltaTime);
		camera.moveRight(velocity.x * deltaTime);
		camera.moveUp(velocity.y * deltaTime);
	}

	camera.applyTransform();
	glMultTransposeMatrixf(e->worldTransform().inverse().getPtr());

	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
}

}	// namespace MCD
