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

CollisionShape::CollisionShape()
	: shapeImpl(nullptr)
{}

CollisionShape::CollisionShape(void* shape)
	: shapeImpl(shape)
{}

CollisionShape::~CollisionShape() {
	delete reinterpret_cast<btCollisionShape*>(shapeImpl);
}

void MCD::intrusivePtrAddRef(CollisionShape* p) {
	++(p->mRefCount);
}

void MCD::intrusivePtrRelease(CollisionShape* p)
{
	if(--(p->mRefCount) == 0)
		delete p;
}

SphereShape::SphereShape(float radius)
	: CollisionShape(new btSphereShape(radius))
{}

StaticPlaneShape::StaticPlaneShape(const Vec3f& planeNormal, float planeConstant)
	: CollisionShape(new btStaticPlaneShape(toBullet(planeNormal), planeConstant))
{}

// TODO: Remove the dependency on opengl
class StaticTriMeshShape::Impl
{
public:
	Impl(const MeshPtr& mesh, void*& shapeImpl)
	{
		MCD_ASSERT(mesh->indexCount() % 3 == 0);

		btIndexedMesh bulletMesh;

		// Get the vertex buffer
		uint vertexHandle = mesh->handle(Mesh::Position);
		glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
		float* vertexBuffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		bulletMesh.m_numVertices = mesh->vertexCount();
		bulletMesh.m_vertexBase = (const unsigned char *)vertexBuffer;
		bulletMesh.m_vertexStride = sizeof(float) * 3;

		// Get the index buffer
		uint indexHandle = mesh->handle(Mesh::Index);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
		uint16_t* indexBuffer = (uint16_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

		bulletMesh.m_numTriangles = mesh->indexCount() / 3;
		bulletMesh.m_triangleIndexBase = (const unsigned char *)indexBuffer;
		bulletMesh.m_triangleIndexStride = sizeof(int16_t) * 3;

		// Assign to bullet
		mBulletVertexIdxArray.addIndexedMesh(bulletMesh, PHY_SHORT);
		shapeImpl = new btBvhTriangleMeshShape(&mBulletVertexIdxArray, true);

		// Unlock the buffers
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	btTriangleIndexVertexArray mBulletVertexIdxArray;
};	// Impl

StaticTriMeshShape::StaticTriMeshShape(const MeshPtr& mesh)
{
	mImpl = new Impl(mesh, shapeImpl);
}

StaticTriMeshShape::~StaticTriMeshShape()
{
	delete mImpl;
}
