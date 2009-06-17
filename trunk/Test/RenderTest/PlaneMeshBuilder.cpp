#include "Pch.h"
#include "PlaneMeshBuilder.h"
#include "../../MCD/Core/Math/Vec2.h"
#include "../../MCD/Core/Math/Mat33.h"
#include "../../MCD/Core/Math/BasicFunction.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../3Party/glew/glew.h"

using namespace MCD;

PlaneMeshBuilder::PlaneMeshBuilder(float width, float height, size_t widthSegmentCount, size_t heightSegmentCount, bool includeTangents)
{
	uint meshFormat = Mesh::Position | Mesh::Normal | Mesh::Index;

	if(includeTangents)
		meshFormat |= Mesh::TextureCoord1;
	else
		meshFormat |= Mesh::TextureCoord0;
	
	enable(meshFormat);

	textureUnit(Mesh::TextureCoord0);
	textureCoordSize(2);

	if(includeTangents)
	{
		textureUnit(Mesh::TextureCoord1);
		textureCoordSize(3);
	}

	const size_t vxCount = widthSegmentCount + 1;	// Number of vertex along x direction
	const size_t vyCount = heightSegmentCount + 1;	// Number of vertex along y direction
	const size_t vertexCount = vxCount * vyCount;		// Number of vertex for single side (1 out of 6) of the cube
	const size_t triCount = 2 * widthSegmentCount * heightSegmentCount;
	
	reserveVertex(vertexCount);
	reserveTriangle(triCount);

	const Vec3f startingCornerXY(-width / 2.0f, 0, -height / 2.0f);
	const Vec3f deltaX(width / widthSegmentCount, 0, 0);
	const Vec3f deltaY(0, 0, height / heightSegmentCount);

	const Vec2f startingCornerUV(-width / 2.0f, -height / 2.0f);
	const Vec2f deltaU(1.0f / widthSegmentCount, 0);
	const Vec2f deltaV(0, 1.0f / heightSegmentCount);

	Vec3f vXY = startingCornerXY;
	Vec2f vUV = startingCornerUV;

	// Create vertices
	for(size_t x = 0; x < vxCount; x++)
	{
		for(size_t y = 0; y < vyCount; y++)
		{
			position(vXY);
			normal(Vec3f(0, 1, 0));
			textureUnit(Mesh::TextureCoord0);
			textureCoord(vUV);
			if(includeTangents)
			{
				textureUnit(Mesh::TextureCoord1);
				textureCoord(Vec3f(0, 0, 1));
			}
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
	for(size_t y = 0; y < heightSegmentCount; y++)
	{
		size_t indexedVertexCount = (y * vxCount);
		for(size_t x = indexedVertexCount; x < indexedVertexCount + widthSegmentCount; x++)
		{       
			addQuad(
				(uint16_t) x,
				(uint16_t)(x + vxCount),
				(uint16_t)(x + vxCount + 1),
				(uint16_t)(x + 1)
				);
		}
	}

}

void PlaneMeshBuilder::commit(Mesh& mesh, StorageHint storageHint)
{
	MeshBuilder::commit(mesh, storageHint);
}
