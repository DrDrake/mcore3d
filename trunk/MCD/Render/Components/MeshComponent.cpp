#include "Pch.h"
#include "MeshComponent.h"
#include "../Effect.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

MeshComponent::MeshComponent() {}

MeshComponent::~MeshComponent() {}

void MeshComponent::render()
{
	Entity* e = entity();
	if(!mesh || !e)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

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
	Entity* e = entity();
	if(!mesh || !e)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

	mesh->drawFaceOnly();

	glPopMatrix();
}

}	// namespace MCD
