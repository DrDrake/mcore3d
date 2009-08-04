#include "Pch.h"
#include "MeshComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Render/Effect.h"
#include "../../Render/Material.h"
#include "../../Render/Mesh.h"
#include "../../Render/Texture.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

MeshComponent::MeshComponent() {}

MeshComponent::~MeshComponent() {}

Component* MeshComponent::clone() const
{
	MeshComponent* cloned = new MeshComponent;
	cloned->mesh = this->mesh;
	cloned->effect = this->effect;
	return cloned;
}

void MeshComponent::render()
{
	Entity* e = entity();
	if(!mesh || !e)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

	Material* material = nullptr;
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

void MeshComponent::render(ICallback& callback)
{
	Entity* e = entity();
	if(!mesh || !e)
		return;

	glPushMatrix();
	glMultTransposeMatrixf(e->worldTransform().getPtr());

	Material* material = nullptr;
	if(effect && (material = effect->material.get()) != nullptr) {
		for(size_t i=0; i<material->getPassCount(); ++i) {
			material->preRender(i);

			callback.preGeomRender(*this);
			mesh->draw();
			callback.postGeomRender(*this);
			
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
