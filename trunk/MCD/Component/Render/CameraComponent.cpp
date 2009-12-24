#include "Pch.h"
#include "CameraComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

CameraComponent::CameraComponent()
	: clearColor(0)
{
}

Component* CameraComponent::clone() const
{
	CameraComponent* cloned = new CameraComponent;
	cloned->camera = this->camera;
	cloned->clearColor = this->clearColor;
	return cloned;
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
