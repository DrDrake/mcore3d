#include "Pch.h"
#include "CameraComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

CameraComponent::CameraComponent()
	: clearColor(0)
{
}

void CameraComponent::render()
{
	Entity* e = entity();
	if(!e)
		return;

	camera.applyTransform();
	glMultTransposeMatrixf(e->worldTransform().inverse().getPtr());

	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
}

}	// namespace MCD
