#include "Pch.h"
#include "TangentSpaceBuilder.h"
#include "Mesh.h"
#include "MeshBuilder.h"

namespace MCD {

namespace {

static void computeTangentBasis(
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

typedef StrideArray<uint16_t> IndexArray;
typedef StrideArray<Vec2f> Vec2fArray;
typedef StrideArray<Vec3f> Vec3fArray;

}	// namespace

bool TangentSpaceBuilder::compute(MeshBuilder& builder, int indexIdx, int posIdx, int normalIdx, int uvIdx, int tangentIdx)
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

	compute(idxPtr, posPtr, nrmPtr, uvPtr,	tangentPtr);

	return true;
}

void TangentSpaceBuilder::compute(
	const IndexArray& indexBuf,
	const Vec3fArray& posBuf,
	const Vec3fArray& nrmBuf,
	const Vec2fArray& uvBuf,
	Vec3fArray& outTangBuf
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

		// Write smoothed tangents back
		outTangBuf[v0] += vT0;
		outTangBuf[v1] += vT1;
		outTangBuf[v2] += vT2;
	}

	// finally normalize the tangents
	for(size_t ivert = 0; ivert < vertexCnt; ++ivert)
		outTangBuf[ivert].normalize();
}

}	// namespace MCD
