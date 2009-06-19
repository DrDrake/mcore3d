#include "Pch.h"
#include "CollisionShape.h"
#include "MathConvertor.inl"
#include "../../Render/Mesh.h"

#include "../../../3Party/bullet/btBulletCollisionCommon.h"
#include "../../../3Party/glew/glew.h"

#ifdef MCD_VC
#	pragma comment(lib, "glew")
#endif	// MCD_VC

using namespace MCD;
using namespace MCD::PhysicsComponent;

CollisionShape::CollisionShape()
	: shapeImpl(nullptr)
{}

CollisionShape::CollisionShape(void* shape)
	: shapeImpl(shape)
{}

CollisionShape::~CollisionShape() {
	delete reinterpret_cast<btCollisionShape*>(shapeImpl);
}

SphereShape::SphereShape(float radius)
	: CollisionShape(new btSphereShape(radius))
{}

StaticPlaneShape::StaticPlaneShape(const Vec3f& planeNormal, float planeConstant)
	: CollisionShape(new btStaticPlaneShape(toBullet(planeNormal), planeConstant))
{}

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

	for(size_t v=0; v < mesh->vertexCount(); ++v)
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

	for(size_t i =  0; i < mesh->indexCount(); ++i)
	{
		btMeshBlder->addIndex(*indexBuffer++);
		if (i % 3 == 0)
			btMeshBlder->getIndexedMeshArray()[0].m_numTriangles++;
	}

	btMeshBlder->getIndexedMeshArray()[0];

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	shapeImpl = new btBvhTriangleMeshShape(btMeshBlder.get(), true);
}
