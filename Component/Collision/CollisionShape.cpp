#include "Pch.h"

#pragma comment(lib, "glew")
#ifdef NDEBUG
#	pragma comment(lib, "MCDRender")
#else
#	pragma comment(lib, "MCDRenderd")
#endif


#include "../../3Party/glew/glew.h"

#include "../MathConvertor.h"
#include "CollisionShape.h"

#include "../../3Party/bullet/btBulletCollisionCommon.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;
using namespace MCD::BulletBinding;

CollisionShape::CollisionShape(btCollisionShape* shape)
{
	mShape.reset(shape);
}

SphereShape::SphereShape(float radius)
{
	mShape.reset(new btSphereShape(radius));
}

StaticPlaneShape::StaticPlaneShape(const Vec3f& planeNormal, float planeConstant)
{
	mShape.reset(new btStaticPlaneShape(MathConvertor::ToBullet(planeNormal), planeConstant));
}

TriMeshShape::TriMeshShape(const MeshPtr& mesh)
{
	// BULLET DESIGN IS VERY ON99
	// THE FOLLOWING CODE MAY NOT WORK...
	// USE btTriangleIndexVertexArray INSTEAD!!!!!!!!!!!!!!!!!!!!!!!!!!!
	std::auto_ptr<btTriangleMesh> btMeshBlder(new btTriangleMesh(false, false));

	btMeshBlder->preallocateVertices(mesh->vertexCount());
	btMeshBlder->preallocateIndices(mesh->indexCount());

	// Add vertices to the bullet triangle mesh
	uint vertexHandle = mesh->handle(Mesh::Position);
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	float* vertexBuffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

	for(size_t v =  0; v < mesh->vertexCount(); v++)
	{
		btVector3 vertex(*vertexBuffer++, *vertexBuffer++, *vertexBuffer++);
		btMeshBlder->findOrAddVertex(vertex, false);
		btMeshBlder->getIndexedMeshArray()[0].m_numVertices++;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Add indices to the bullet triangle mesh
	uint indexHandle = mesh->handle(Mesh::Index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	uint16_t* indexBuffer = (uint16_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

	for(size_t i =  0; i < mesh->indexCount(); i++)
	{
		btMeshBlder->addIndex(*indexBuffer++);
		if (i % 3 == 0)
			btMeshBlder->getIndexedMeshArray()[0].m_numTriangles++;
	}

	 btMeshBlder->getIndexedMeshArray()[0]

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mShape.reset(new btBvhTriangleMeshShape(btMeshBlder.get(), true));
}
