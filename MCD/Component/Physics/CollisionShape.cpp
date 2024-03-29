#include "Pch.h"
#include "CollisionShape.h"
#include "MathConvertor.inl"
#include "../../Core/System/Log.h"
#include "../../Render/Mesh.h"
#include "../../Render/MeshBuilder.h"

#include "../../../3Party/bullet/btBulletCollisionCommon.h"

using namespace MCD;

typedef StrideArray<uint16_t> IndexArray;
typedef StrideArray<Vec3f> Vec3fArray;

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
		Mesh::MappedBuffers mapped;
		StrideArray<Vec3f> vertex = mesh->mapAttribute<Vec3f>(Mesh::cPositionAttrIdx, mapped, Mesh::Read);
		StrideArray<uint16_t> index = mesh->mapAttribute<uint16_t>(Mesh::cIndexAttrIdx, mapped, Mesh::Read);

		init(vertex, index, true, shapeImpl);

		// Unlock the buffers
		mesh->unmapBuffers(mapped);
	}

	Impl(MeshBuilder& meshBuilder, int positionId, bool keepOwnBuffer, void*& shapeImpl)
		: mVertexBuffer(nullptr), mIndexBuffer(nullptr)
	{
		const IndexArray idxPtr = meshBuilder.getAttributeAs<uint16_t>(0);
		const Vec3fArray posPtr = meshBuilder.getAttributeAs<Vec3f>(positionId);

		if(idxPtr.data && posPtr.data)
			init(posPtr, idxPtr, keepOwnBuffer, shapeImpl);
		else {
			Log::write(Log::Error, "An empty mesh is passed into StaticTriMeshShape constructor");
			shapeImpl = nullptr;
		}
	}

	void init(
		Vec3fArray vertexBuffer, IndexArray indexBuffer,
		bool keepOwnBuffer, void*& shapeImpl)
	{
		MCD_ASSERT(indexBuffer.size % 3 == 0);

		if(keepOwnBuffer) {
			mVertexBuffer = new Vec3f[vertexBuffer.size];
			mIndexBuffer = new uint16_t[indexBuffer.size];

			for(size_t i=0; i<vertexBuffer.size; ++i)
				mVertexBuffer[i] = vertexBuffer[i];
			for(size_t i=0; i<indexBuffer.size; ++i)
				mIndexBuffer[i] = indexBuffer[i];

			vertexBuffer = Vec3fArray(mVertexBuffer, vertexBuffer.size);
			indexBuffer = IndexArray(mIndexBuffer, indexBuffer.size);
		}

		btIndexedMesh bulletMesh;

		bulletMesh.m_numVertices = vertexBuffer.size;
		bulletMesh.m_vertexBase = (const unsigned char *)vertexBuffer.data;
		bulletMesh.m_vertexStride = sizeof(float) * 3;

		bulletMesh.m_numTriangles = indexBuffer.size / 3;
		bulletMesh.m_triangleIndexBase = (const unsigned char *)indexBuffer.data;
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
	: mImpl(*new Impl(mesh, shapeImpl))
{
}

StaticTriMeshShape::StaticTriMeshShape(const MeshBuilder& meshBuilder,int positionId, bool keepOwnBuffer)
	: mImpl(*new Impl(const_cast<MeshBuilder&>(meshBuilder), positionId, keepOwnBuffer, shapeImpl))
{
}

StaticTriMeshShape::~StaticTriMeshShape()
{
	delete &mImpl;
}
