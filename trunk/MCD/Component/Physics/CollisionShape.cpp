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
	std::vector<float> mVertexBuffer;
	std::vector<int> mIndexBuffer;
	std::auto_ptr<btTriangleIndexVertexArray> mBulletVertexIdxArray;

public:
	Impl(const MeshPtr& mesh, void*& shapeImpl)
	{
		// Firstly, copy the vertex buffer and index buffer from OpenGL buffer to system memory.
		// Then, pass them into btTriangleIndexVertexArray
		// The array of vertex and index data should be freed by StaticTriMeshShape
		assert(mesh->indexCount() % 3 == 0);

		mVertexBuffer.resize(mesh->vertexCount() * 3);
		mIndexBuffer.resize(mesh->indexCount());

		// Copy the vertex buffer
		uint vertexHandle = mesh->handle(Mesh::Position);
		glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
		float* vertexBuffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		memcpy(&mVertexBuffer[0], vertexBuffer, mVertexBuffer.size() * sizeof(float));

		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Copy the index buffer
		uint indexHandle = mesh->handle(Mesh::Index);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
		uint16_t* indexBuffer = (uint16_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

		// The index buffer cannot be copied directly. 16 bit index is used in OpenGL, where 32 bit index is used in Bullet
		for(size_t idx = 0; idx < mesh->indexCount(); idx++)
			mIndexBuffer[idx] = indexBuffer[idx];

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Keep it alive until shutdown
		mBulletVertexIdxArray.reset(new btTriangleIndexVertexArray);

		btIndexedMesh bulletMesh;
		// vertex
		bulletMesh.m_numVertices = mVertexBuffer.size();
		bulletMesh.m_vertexBase = (unsigned char *)&mVertexBuffer[0];
		bulletMesh.m_vertexStride = sizeof(float) * 3;
		// index
		bulletMesh.m_indexType = PHY_INTEGER;
		bulletMesh.m_numTriangles = mIndexBuffer.size() / 3;
		bulletMesh.m_triangleIndexBase = (unsigned char *)&mIndexBuffer[0];
		bulletMesh.m_triangleIndexStride = sizeof(int) * 3;
		
		mBulletVertexIdxArray->addIndexedMesh(bulletMesh);

		shapeImpl = new btBvhTriangleMeshShape(mBulletVertexIdxArray.get(), true);
	}
};

StaticTriMeshShape::StaticTriMeshShape(const MeshPtr& mesh)
{
	mImpl = new Impl(mesh, shapeImpl);
}

StaticTriMeshShape::~StaticTriMeshShape()
{
	delete mImpl;
}
