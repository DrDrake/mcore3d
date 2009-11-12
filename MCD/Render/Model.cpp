#include "Pch.h"
#include "Model.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"	// TODO: Remove this dependency
#include "../Core/System/Utility.h"

namespace MCD {

Model::MeshAndMaterial::MeshAndMaterial()
{
}

Model::MeshAndMaterial::~MeshAndMaterial()
{
}

Model::~Model()
{
}

void Model::draw()
{
	for(MeshAndMaterial* meshAndMat = mMeshes.begin(); meshAndMat != mMeshes.end(); meshAndMat = meshAndMat->next())
	{
		if(!meshAndMat->mesh || !meshAndMat->effect)
			continue;

		Material* material = meshAndMat->effect->material.get();
		if(!material)
			continue;

		size_t passCount = material->getPassCount();
		for(size_t passId = 0; passId < passCount; ++passId)
		{
			material->preRender(passId);
			meshAndMat->mesh->draw();
			material->postRender(passId);
		}
	}
}

}	// namespace MCD
