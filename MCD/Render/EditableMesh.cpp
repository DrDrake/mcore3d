#include "Pch.h"

#include "EditableMesh.h"
#include "MeshBuilder.h"	// For ~Mesh() to work

namespace MCD {

EditableMesh::EditableMesh(const Path& fileId)
	: Mesh(fileId)
{
}

EditableMesh::EditableMesh(const Path& fileId, const EditableMesh& shareBuffer)
	: Mesh(fileId, shareBuffer)
{
	this->builder = shareBuffer.builder;
}

}	// namespace MCD