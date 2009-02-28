#include "Pch.h"
#include "MeshComponent.h"
#include "../Effect.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

MeshComponent::~MeshComponent()
{
}

void MeshComponent::render()
{
	if(!mesh)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(entity()->worldTransform().getPtr());

	Material2* material = nullptr;
	if(effect && (material = effect->material.get()) != nullptr) {
		for(size_t i=0; i<material->getPassCount(); ++i) {
			material->preRender(i);
			mesh->draw();
			material->postRender(i);
		}
	}
	else
		mesh->drawFaceOnly();

	glPopMatrix();
}

void MeshComponent::renderFaceOnly()
{
	if(!mesh)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(entity()->worldTransform().getPtr());

	mesh->drawFaceOnly();

	glPopMatrix();
}

}	// namespace MCD
