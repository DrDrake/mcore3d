#include "Pch.h"
#include "CollisionShape.h"
#include "MathConvertor.inl"
#include "../../Core/System/Log.h"
#include "../../Render/Mesh.h"
#include "../../Render/MeshBuilder.h"

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

SphereShape::SphereShape(float radius)
	: CollisionShape(new btSphereShape(radius))
{}

StaticPlaneShape::StaticPlaneShape(const Vec3f& planeNormal, float planeConstant)
	: CollisionShape(new btStaticPlaneShape(toBullet(planeNormal), planeConstant))
{}

class StaticTriMeshShape::Impl
{
public:
	Impl(const MeshPtr& mesh, void*& shapeImpl) : mVertexBuffer(nullptr), mIndexBuffer(nullptr)
	{
		// Get the vertex buffer
		uint vertexHandle = mesh->handle(Mesh::Position);
		size_t vertexCount = mesh->vertexCount();
		glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
		const Vec3f* vertexBuffer = reinterpret_cast<Vec3f*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));

		// Get the index buffer
		uint indexHandle = mesh->handle(Mesh::Index);
		size_t indexCount = mesh->indexCount();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
		const uint16_t* indexBuffer = reinterpret_cast<uint16_t*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));

		init(vertexBuffer, indexBuffer, vertexCount, indexCount, true, shapeImpl);

		// Unlock the buffers
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Impl(MeshBuilder& meshBuilder, bool keepOwnBuffer, void*& shapeImpl) : mVertexBuffer(nullptr), mIndexBuffer(nullptr)
	{
		size_t vertexCount = 0;
		const Vec3f* vertexBuffer = reinterpret_cast<Vec3f*>(meshBuilder.acquireBufferPointer(Mesh::Position, &vertexCount));
		// TODO: if(vertexCount == 0)

		size_t indexCount = 0;
		uint16_t* indexBuffer = reinterpret_cast<uint16_t*>(meshBuilder.acquireBufferPointer(Mesh::Index, &indexCount));
		// TODO: if(indexCount == 0)

		if(vertexCount > 0 && indexCount > 0 && vertexBuffer && indexBuffer)
			init(vertexBuffer, indexBuffer, vertexCount, indexCount, keepOwnBuffer, shapeImpl);
		else {
			Log::write(Log::Error, L"An empty mesh is passed into StaticTriMeshShape constructor");
			shapeImpl = nullptr;
		}
	}

	void init(
		sal_in_ecount(vertexCount) const Vec3f* vertexBuffer, sal_in_ecount(indexCount) const uint16_t* indexBuffer,
		size_t vertexCount, size_t indexCount, bool keepOwnBuffer, void*& shapeImpl)
	{
		MCD_ASSERT(indexCount % 3 == 0);

		if(keepOwnBuffer) {
			mVertexBuffer = new Vec3f[vertexCount];
			mIndexBuffer = new uint16_t[indexCount];
			::memcpy(mVertexBuffer, vertexBuffer, vertexCount * sizeof(Vec3f));
			::memcpy(mIndexBuffer, indexBuffer, indexCount * sizeof(uint16_t));
			vertexBuffer = mVertexBuffer;
			indexBuffer = mIndexBuffer;
		}

		btIndexedMesh bulletMesh;

		bulletMesh.m_numVertices = vertexCount;
		bulletMesh.m_vertexBase = (const unsigned char *)vertexBuffer;
		bulletMesh.m_vertexStride = sizeof(float) * 3;

		bulletMesh.m_numTriangles = indexCount / 3;
		bulletMesh.m_triangleIndexBase = (const unsigned char *)indexBuffer;
		bulletMesh.m_triangleIndexStride = sizeof(int16_t) * 3;

		// Assign to bullet
		mBulletVertexIdxArray.addIndexedMesh(bulletMesh, PHY_SHORT);
		shapeImpl = new btBvhTriangleMeshShape(&mBulletVertexIdxArray, true, true);	// bool useQuantizedAabbCompression, bool buildBvh
	}

	~Impl()
	{
		delete[] mVertexBuffer;
		delete[] mIndexBuffer;
	}

	btTriangleIndexVertexArray mBulletVertexIdxArray;
	Vec3f* mVertexBuffer;
	uint16_t* mIndexBuffer;
};	// Impl

StaticTriMeshShape::StaticTriMeshShape(const MeshPtr& mesh)
{
	mImpl = new Impl(mesh, shapeImpl);
}

StaticTriMeshShape::StaticTriMeshShape(const MeshBuilder& meshBuilder, bool keepOwnBuffer)
{
	mImpl = new Impl(const_cast<MeshBuilder&>(meshBuilder), keepOwnBuffer, shapeImpl);
}

StaticTriMeshShape::~StaticTriMeshShape()
{
	delete mImpl;
}
