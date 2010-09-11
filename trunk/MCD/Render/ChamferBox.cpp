#include "Pch.h"
#include "ChamferBox.h"
#include "MeshBuilder.h"
#include "TangentSpaceBuilder.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Mat33.h"
#include "../Core/Math/BasicFunction.h"

using namespace MCD;

ChamferBoxBuilder::ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount, bool includeTangents)
{
	posId = declareAttribute(VertexFormat::get("position"), 1);
	normalId = declareAttribute(VertexFormat::get("normal"), 1);
	uvId = declareAttribute(VertexFormat::get("uv0"), 1);

	tangentId = -1;
	if(includeTangents)
		tangentId = declareAttribute(VertexFormat::get("tangent"), 1);

	const size_t cubeFaceCount = 6;
	const size_t rowCount = (filletSegmentCount + 1) * 2;	// Number of vertex along y direction
	const size_t columnCount = rowCount;					// Number of vertex along x direction
	const size_t sliceCount = filletSegmentCount * 2 + 1;	// Number of triangles for single side (1 out of 6) of the cube
	const size_t vertexCount = rowCount * columnCount;		// Number of vertex for single side (1 out of 6) of the cube
	const size_t indexCount = 3 * 2 * sliceCount * sliceCount;
	const float segmentWidth = (filletSegmentCount == 0) ? 0 : filletRadius / filletSegmentCount;

	const Vec3f center(0, 0, 0);
	const Vec3f extent(1.0f);

	MCD_VERIFY(reserveBuffers(uint16_t(vertexCount * cubeFaceCount), indexCount * cubeFaceCount));

	const Array<Mat33f, cubeFaceCount> transforms = {{
		Mat33f::cIdentity,
		Mat33f::makeXYZRotation(0, Math<float>::cPiOver2() * 1, 0),
		Mat33f::makeXYZRotation(0, Math<float>::cPiOver2() * 2, 0),
		Mat33f::makeXYZRotation(0, Math<float>::cPiOver2() * 3, 0),
		Mat33f::makeXYZRotation(Math<float>::cPiOver2() * 1, 0, 0),
		Mat33f::makeXYZRotation(Math<float>::cPiOver2() * 3, 0, 0),
	}};

	// Loop for the 6 faces of the unit cube
	for(size_t cubeFace = 0; cubeFace < cubeFaceCount; ++cubeFace)
	{
		Vec3f position(0, 0, extent.z);
		Vec3f corner(0, 0, extent.z - filletRadius);

		for(size_t x = 0; x < columnCount; ++x)
		{
			if(x <= filletSegmentCount) {
				corner.x = center.x - extent.x + filletRadius;
				position.x = corner.x - filletRadius + x * segmentWidth;
			}
			else {
				corner.x = center.x + extent.x - filletRadius;
				position.x = corner.x + (x - filletSegmentCount - 1) * segmentWidth;
			}

			for(size_t y = 0; y < rowCount; ++y)
			{
				if(y <= filletSegmentCount) {
					corner.y = center.y - extent.y + filletRadius;
					position.y = corner.y - filletRadius + y * segmentWidth;
				}
				else {
					corner.y = center.y + (extent.y - filletRadius);
					position.y = corner.y + (y - filletSegmentCount - 1) * segmentWidth;
				}

				const Vec3f diff = position - corner;
				Vec3f p(corner), n(0, 0, 1);

				if(diff.length() > 0) {
					// Normalize the vector length to form the curve
					p += diff * (filletRadius / (position - corner).length());
					n = diff.normalizedCopy();
				}

				p = transforms[cubeFace] * p;
				n = transforms[cubeFace] * n;

				const Vec2f uv = 0.5f * Vec2f(position.x / extent.x, position.y / extent.y) - 0.5f;

				MCD_VERIFY(vertexAttribute(posId, &p));
				MCD_VERIFY(vertexAttribute(normalId, &n));
				MCD_VERIFY(vertexAttribute(uvId, &uv));
				// We will calculate the tangent using TangentSpaceBuilder later on.

				MCD_VERIFY(addVertex() != uint16_t(-1));
			}
		}
	}

	for(size_t cubeFace = 0; cubeFace < cubeFaceCount; ++cubeFace)
	{
		// Index
		//  y ^
		//    |   i3
		// i2 +---+
		//    |\  |
		//    | \ |
		//    |  \|
		//    +---+-> x
		//   i0   i1

		// Face order:
		// 1: i0, i3, i1
		// 2: i0, i2, i3
		for(size_t y = 0; y < sliceCount; ++y)
		{
			const uint16_t startingIndex = uint16_t(vertexCount * cubeFace);
			uint16_t i0 = uint16_t(startingIndex + columnCount * y);
			uint16_t i1 = i0 + 1;
			uint16_t i2 = i0 + uint16_t(columnCount);
			uint16_t i3 = i2 + 1;

			for(size_t x = 0; x < sliceCount; ++x)
			{
				MCD_VERIFY(addTriangle(i1, i0, i2));
				MCD_VERIFY(addTriangle(i2, i3, i1));

				++i0;
				++i1;
				++i2;
				++i3;
			}
		}
	}

	if(includeTangents)
		MCD_VERIFY(TangentSpaceBuilder().compute(*this, 0,	posId, normalId, uvId, tangentId));
}
