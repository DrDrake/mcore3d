#include "Pch.h"
#include "SphereBuilder.h"
#include "MeshBuilder.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Mat33.h"
#include "../Core/Math/BasicFunction.h"

using namespace MCD;

SphereBuilder::SphereBuilder(float radius, uint16_t segmentCount)
{
	const uint16_t widthSegmentCount = segmentCount;
	const uint16_t heightSegmentCount = segmentCount;

	// Many code are borrowed from PlanMeshBuilder
	posId = declareAttribute(VertexFormat::get("position"), 1);
	normalId = declareAttribute(VertexFormat::get("normal"), 1);
	uvId = declareAttribute(VertexFormat::get("uv0"), 1);

	const uint16_t vxCount = widthSegmentCount + 1;		// Number of vertex along x direction
	const uint16_t vzCount = heightSegmentCount + 1;	// Number of vertex along z direction
	const uint16_t vertexCount = vxCount * vzCount;		// Number of vertex for the whole plane
	const uint16_t triCount = 2 * widthSegmentCount * heightSegmentCount;

	MCD_VERIFY(reserveBuffers(vertexCount, triCount * 3));

	// Create vertices
	for(uint16_t x = 0; x < vxCount; ++x)
	{
		for(uint16_t z = 0; z < vzCount; ++z)
		{
			// Calculate the 2 angles by mapping [0 - segment count] to [0 to 2PI]
			const float ax = 2 * Mathf::cPi() * float(x) / widthSegmentCount;
			const float az = 1 * Mathf::cPi() * float(z) / heightSegmentCount + Mathf::cPiOver2();

			Vec3f normal(
				cosf(ax) * cosf(az),
				sinf(az),
				sinf(ax) * cosf(az)
			);

			Vec3f pos = normal * radius;

			const Vec2f uv(1 - float(x) / (vxCount-1), 1 - float(z) / (vzCount-1));
			MCD_VERIFY(vertexAttribute(posId, &pos));
			MCD_VERIFY(vertexAttribute(normalId, &normal));
			MCD_VERIFY(vertexAttribute(uvId, &uv));

			addVertex();
		}
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
