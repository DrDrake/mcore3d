#include "Pch.h"
#include "PlaneMeshBuilder.h"
#include "SemanticMap.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"

using namespace MCD;

PlaneMeshBuilder::PlaneMeshBuilder(float width, float height, uint16_t widthSegmentCount, uint16_t heightSegmentCount, bool includeTangents)
{
	int posId = declareAttribute(SemanticMap::getSingleton().position(), 1);
	int normalId = declareAttribute(SemanticMap::getSingleton().normal(), 1);
	int uvId = declareAttribute(SemanticMap::getSingleton().uv(0, 2), 1);
	int tangentId = -1;

	if(includeTangents)
		tangentId = declareAttribute(SemanticMap::getSingleton().tangent(), 1);

	const uint16_t vxCount = widthSegmentCount + 1;	// Number of vertex along x direction
	const uint16_t vyCount = heightSegmentCount + 1;	// Number of vertex along y direction
	const uint16_t vertexCount = vxCount * vyCount;	// Number of vertex for the whole plane
	const uint16_t triCount = 2 * widthSegmentCount * heightSegmentCount;

	reserveBuffers(vertexCount, triCount * 3);

	const Vec3f startingCornerXY(-width / 2.0f, 0, -height / 2.0f);
	const Vec3f deltaX(width / widthSegmentCount, 0, 0);
	const Vec3f deltaY(0, 0, height / heightSegmentCount);

	const Vec2f startingCornerUV(-width / 2.0f, -height / 2.0f);
	const Vec2f deltaU(1.0f / widthSegmentCount, 0);
	const Vec2f deltaV(0, 1.0f / heightSegmentCount);

	Vec3f vXY = startingCornerXY;
	Vec2f vUV = startingCornerUV;

	// Create vertices
	for(uint16_t x = 0; x < vxCount; ++x)
	{
		for(uint16_t y = 0; y < vyCount; ++y)
		{
			vertexAttribute(posId, &vXY);
			vertexAttribute(normalId, &Vec3f::c010);
			vertexAttribute(uvId, &vUV);
			if(includeTangents)
				vertexAttribute(tangentId, &Vec3f::c001);

			addVertex();
			vXY += deltaX;
			vUV += deltaU;
		}
		vXY.x = startingCornerXY.x;
		vUV.x = startingCornerUV.x;
		vXY += deltaY;
		vUV += deltaV;
	}

	// Create index
	for(uint16_t y = 0; y < heightSegmentCount; ++y)
	{
		uint16_t indexedVertexCount = (y * vxCount);
		for(uint16_t x = indexedVertexCount; x < indexedVertexCount + widthSegmentCount; ++x)
		{       
			addQuad(
				x,
				x + vxCount,
				x + vxCount + 1,
				x + 1
			);
		}
	}
}
