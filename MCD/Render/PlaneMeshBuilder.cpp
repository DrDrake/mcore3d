#include "Pch.h"
#include "PlaneMeshBuilder.h"
#include "SemanticMap.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"

using namespace MCD;

PlaneMeshBuilder::PlaneMeshBuilder(float width, float height, uint16_t widthSegmentCount, uint16_t heightSegmentCount, bool includeTangents)
{
	posId = declareAttribute(SemanticMap::getSingleton().position(), 1);
	normalId = declareAttribute(SemanticMap::getSingleton().normal(), 1);
	uvId = declareAttribute(SemanticMap::getSingleton().uv(0, 2), 1);
	tangentId = -1;

	if(includeTangents)
		tangentId = declareAttribute(SemanticMap::getSingleton().tangent(), 1);

	const uint16_t vxCount = widthSegmentCount + 1;		// Number of vertex along x direction
	const uint16_t vzCount = heightSegmentCount + 1;	// Number of vertex along z direction
	const uint16_t vertexCount = vxCount * vzCount;		// Number of vertex for the whole plane
	const uint16_t triCount = 2 * widthSegmentCount * heightSegmentCount;

	MCD_VERIFY(reserveBuffers(vertexCount, triCount * 3));

	const Vec3f startingCornerXZ(-width / 2.0f, 0, -height / 2.0f);
	const Vec3f deltaX(width / widthSegmentCount, 0, 0);
	const Vec3f deltaZ(0, 0, height / heightSegmentCount);

	const Vec2f startingCornerUV(-width / 2.0f, -height / 2.0f);
	const Vec2f deltaU(1.0f / widthSegmentCount, 0);
	const Vec2f deltaV(0, 1.0f / heightSegmentCount);

	Vec3f vXZ = startingCornerXZ;
	Vec2f vUV = startingCornerUV;

	// Create vertices
	for(uint16_t z = 0; z < vzCount; ++z)
	{
		for(uint16_t x = 0; x < vxCount; ++x)
		{
			MCD_VERIFY(vertexAttribute(posId, &vXZ));
			MCD_VERIFY(vertexAttribute(normalId, &Vec3f::c010));
			MCD_VERIFY(vertexAttribute(uvId, &vUV));
			if(includeTangents)
				MCD_VERIFY(vertexAttribute(tangentId, &Vec3f::c001));

			addVertex();
			vXZ += deltaX;
			vUV += deltaU;
		}
		vXZ.x = startingCornerXZ.x;
		vUV.x = startingCornerUV.x;
		vXZ += deltaZ;
		vUV += deltaV;
	}

	// Create index
	for(uint16_t z = 0; z < heightSegmentCount; ++z)
	{
		uint16_t indexedVertexCount = (z * vxCount);
		for(uint16_t x = indexedVertexCount; x < indexedVertexCount + widthSegmentCount; ++x)
		{       
			MCD_VERIFY(addQuad(
				x,
				x + vxCount,
				x + vxCount + 1,
				x + 1
			));
		}
	}

	MCD_ASSERT(indexCount() / 3 == triCount);
}
