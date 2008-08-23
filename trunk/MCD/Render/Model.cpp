#include "Pch.h"
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"
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
