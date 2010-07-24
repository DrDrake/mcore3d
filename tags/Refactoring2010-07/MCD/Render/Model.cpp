#include "Pch.h"
#include "Model.h"
#include "Effect.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshBuilder.h"
#include "../Core/System/Utility.h"

namespace MCD {

Model::MeshAndMaterial::MeshAndMaterial() {}

Model::MeshAndMaterial::~MeshAndMaterial() {}

Model::Model(const Path& fileId) : Resource(fileId) {}

Model::~Model() {}

void Model::draw()
{
	for(MeshAndMaterial* meshAndMat = mMeshes.begin(); meshAndMat != mMeshes.end(); meshAndMat = meshAndMat->next())
	{
		if(!meshAndMat->mesh)
			continue;

		if(meshAndMat->effect) {
			Material* material = meshAndMat->effect->material.get();
			if(!material) {
				meshAndMat->mesh->draw();
				continue;
			}

			size_t passCount = material->getPassCount();
			for(size_t passId = 0; passId < passCount; ++passId)
			{
				material->preRender(passId);
				meshAndMat->mesh->draw();
				material->postRender(passId);
			}
		}
		else
			meshAndMat->mesh->draw();
	}
}

}	// namespace MCD
