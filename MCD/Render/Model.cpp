#include "Pch.h"
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"	// TODO: Remove this dependency
#include "../Core/System/Utility.h"

namespace MCD {

void Model::draw()
{
	MCD_FOREACH(const MeshAndMaterial& mesh, mMeshes) {
		mesh.material.bind();
		mesh.mesh->draw();
	}
}

}	// namespace MCD
