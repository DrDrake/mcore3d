#include "Pch.h"
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"	// TODO: Remove this dependency
#include "../Core/System/Utility.h"

namespace MCD {

Model::MeshAndMaterial::MeshAndMaterial()
	: material(nullptr)
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
		if(!meshAndMat->mesh || !meshAndMat->material.get())
			continue;

		size_t passCount = meshAndMat->material->getPassCount();
		for(size_t passId = 0; passId < passCount; ++passId)
		{
			meshAndMat->material->preRender(passId);
			meshAndMat->mesh->draw();
			meshAndMat->material->postRender(passId);
		}
	}
}

}	// namespace MCD
