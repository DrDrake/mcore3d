#include "Pch.h"
#include "MeshComponent.h"
#include "../Effect.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Texture.h"

namespace MCD {

MeshComponent::~MeshComponent()
{
}

void MeshComponent::render()
{
	Material2* material = nullptr;
	if(effect && (material = effect->material.get()) != nullptr) {
		for(size_t i=0; i<material->getPassCount(); ++i) {
			material->preRender(i);
			mesh->draw();
			material->postRender(i);
		}
	}
}

}	// namespace MCD
