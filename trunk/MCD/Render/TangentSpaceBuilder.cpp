#include "Pch.h"
#include "TangentSpaceBuilder.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "../Core/System/Array.h"

namespace MCD {

namespace {

// TDOO: May move this to MeshBuilder.h
/*! A small utility which acquire the mesh buffer pointer in its constructor;
	and release the pointer in its destructor.
 */
class MeshBufferPointer
{
public:
	MeshBufferPointer(MeshBuilder& builder, int dataType)
		: mBuilder(builder)
	{
		mDataPtr = builder.acquireBufferPointer(dataType, &mDataSize);
	}

	~MeshBufferPointer()
	{
		mBuilder.releaseBufferPointer(mDataPtr);
	}

	template<typename T>
	T* as()
	{	// TODO: Add memory size checking eg. sizeof(T) == getSize(meshDataType)
		return (T*)mDataPtr;
	}

	size_t size() const { return mDataSize; }

private:
	MeshBuilder&	mBuilder;
	void*			mDataPtr;
	size_t			mDataSize;	//! The number of elements (not memory size!)
};	// MeshBufferPointer

void computeTangentBasis(
	const Vec3f& P1, const Vec3f& P2, const Vec3f& P3, 
	const Vec2f& UV1, const Vec2f& UV2, const Vec2f& UV3,
	Vec3f& tangent, Vec3f& bitangent
	)
{
	// reference: http://www.3dkingdoms.com/weekly/weekly.php?a=37
	const Vec3f Edge1 = P2 - P1;
	const Vec3f Edge2 = P3 - P1;
	const Vec2f Edge1uv = UV2 - UV1;
	const Vec2f Edge2uv = UV3 - UV1;

	const float cp = Edge1uv.y * Edge2uv.x - Edge1uv.x * Edge2uv.y;

	if ( cp != 0.0f )
	{
		float mul = 1.0f / cp;
		tangent = (Edge1 * -Edge2uv.y + Edge2 * Edge1uv.y) * mul;
		bitangent = (Edge1 * -Edge2uv.x + Edge2 * Edge1uv.x) * mul;

		tangent.normalize();
		bitangent.normalize();
	}
}

typedef ArrayWrapper<uint16_t> IndexArray;
typedef ArrayWrapper<Vec2f> Vec2fArray;
typedef ArrayWrapper<Vec3f> Vec3fArray;

void compute(
	const IndexArray& indexBuf, const Vec3fArray& posBuf,
	const Vec3fArray& nrmBuf, const Vec2fArray& uvBuf, Vec3fArray& outTangBuf
)
{
	MCD_ASSERT(posBuf.size == nrmBuf.size && uvBuf.size == outTangBuf.size && posBuf.size == outTangBuf.size);

	const size_t vertexCnt = posBuf.size;
	const size_t faceCnt = indexBuf.size / 3;

	// Initialize tangents to zero-length vectors
	for(size_t ivert = 0; ivert < vertexCnt; ++ivert)
		outTangBuf[ivert] = Vec3f::cZero;

	// Compute tangent for each face and add to each corresponding vertex
	for(size_t iface = 0; iface < faceCnt; ++iface)
	{
		const uint16_t v0 = indexBuf[iface*3+0];
		const uint16_t v1 = indexBuf[iface*3+1];
		const uint16_t v2 = indexBuf[iface*3+2];

		MCD_ASSERT(v0 < vertexCnt);
		MCD_ASSERT(v1 < vertexCnt);
		MCD_ASSERT(v2 < vertexCnt);

		const Vec3f& vN0 = nrmBuf[v0];
		const Vec3f& vN1 = nrmBuf[v1];
		const Vec3f& vN2 = nrmBuf[v2];

		Vec3f vT, vB;

		computeTangentBasis
			( posBuf[v0], posBuf[v1], posBuf[v2]
			, uvBuf[v0], uvBuf[v1], uvBuf[v2]
			, vT, vB );

		Vec3f vT0 = (vT - vN0 * vT.dot(vN0));
		Vec3f vT1 = (vT - vN1 * vT.dot(vN1));
		Vec3f vT2 = (vT - vN2 * vT.dot(vN2));

		vT0.normalize();
		vT1.normalize();
		vT2.normalize();

		//Vec3f vB0 = vT0.cross(vN0);
		//Vec3f vB1 = vT1.cross(vN1);
		//Vec3f vB2 = vT2.cross(vN2);
		
		// Write smoothed tangents back
		outTangBuf[v0] += vT0;
		outTangBuf[v1] += vT1;
		outTangBuf[v2] += vT2;
	}

	// finally normalize the tangents
	for(size_t ivert = 0; ivert < vertexCnt; ++ivert)
		outTangBuf[ivert].normalize();
}

}	// namespace

bool TangentSpaceBuilder::compute(MeshBuilder& builder, int uvDataType, int tangentDataType)
{
	const uint cCurrentFormat = builder.format();

	if(0 == (cCurrentFormat & tangentDataType))
		return false;

	// acquire mesh buffer pointers
	MeshBufferPointer xyzPtr(builder, Mesh::Position);
	MeshBufferPointer nrmPtr(builder, Mesh::Normal);
	MeshBufferPointer uvPtr(builder, (Mesh::DataType)uvDataType);
	MeshBufferPointer tangPtr(builder, (Mesh::DataType)tangentDataType);

	// make sure the data
	if(xyzPtr.size() != uvPtr.size() || xyzPtr.size() != tangPtr.size())
		return false;

	MeshBufferPointer idxPtr(builder, Mesh::Index);

	// we supports triangles only :-)
	const size_t cFaceCnt	= idxPtr.size() / 3;
	const size_t cVertexCnt = xyzPtr.size();

	compute(
		cFaceCnt, cVertexCnt,
		idxPtr.as<uint16_t>(), xyzPtr.as<Vec3f>(), nrmPtr.as<Vec3f>(), uvPtr.as<Vec2f>(),
		tangPtr.as<Vec3f>()
	);

	return true;
}

bool TangentSpaceBuilder::compute(MeshBuilder2& builder, int indexIdx, int posIdx, int normalIdx, int uvIdx, int tangentIdx)
{
	const IndexArray idxPtr = builder.getAttributeAs<uint16_t>(indexIdx);
	const Vec3fArray posPtr = builder.getAttributeAs<Vec3f>(posIdx);
	const Vec3fArray nrmPtr = builder.getAttributeAs<Vec3f>(normalIdx);
	const Vec2fArray uvPtr = builder.getAttributeAs<Vec2f>(uvIdx);
	Vec3fArray tangentPtr = builder.getAttributeAs<Vec3f>(tangentIdx);

	if(!idxPtr.data || !posPtr.data || !nrmPtr.data || !uvPtr.data || !tangentPtr.data)
		return false;

	if(idxPtr.size != builder.indexCount())
		return false;
	if(posPtr.size != builder.vertexCount())
		return false;

	MCD::compute(idxPtr, posPtr, nrmPtr, uvPtr,	tangentPtr);

	return true;
}

void TangentSpaceBuilder::compute(
	const size_t faceCnt, const size_t vertexCnt,
	const uint16_t* indexBuf, const Vec3f* posBuf, const Vec3f* nrmBuf, const Vec2f* uvBuf,
	Vec3f* outTangBuf
	)
{
	Vec3fArray result(outTangBuf, vertexCnt);
	MCD::compute(
		IndexArray((void*)indexBuf, faceCnt*3), Vec3fArray((void*)posBuf, vertexCnt),
		Vec3fArray((void*)nrmBuf, vertexCnt), Vec2fArray((void*)uvBuf, vertexCnt),
		result
	);
}

}	// namespace MCD