#include "../../MCD/Render/GlWindow.h"
#include "../../MCD/Core/Math/Mat33.h"
#include "../../MCD/Core/Math/BasicFunction.h"
#include "../../3Party/glew/glew.h"

#if defined(MCD_VC)
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "GlAux")
#endif

//! Create a chamfer box with length = 2
class ChamferBoxBuilder : protected MeshBuilder
{
public:
	using MeshBuilder::StorageHint;

	ChamferBoxBuilder(float filletRadius, size_t filletSegmentCount)
	{
		begin(Mesh::Position | Mesh::Normal | Mesh::Index);

		const size_t cubeFaceCount = 6;
		const size_t rowCount = (filletSegmentCount + 1) * 2;	// Number of vertex along y direction
		const size_t columnCount = rowCount;					// Number of vertex along x direction
		const size_t sliceCount = filletSegmentCount * 2 + 1;	// Number of triangles for single side (1 out of 6) of the cube
		const size_t vertexCount = rowCount * columnCount;		// Number of vertex for single side (1 out of 6) of the cube
		const size_t indexCount = 3 * 2 * sliceCount * sliceCount;
		const float segmentWidth = (filletSegmentCount == 0) ? 0 : filletRadius / filletSegmentCount;

		const Vec3f center(0, 0, 0);
		const Vec3f extend(1.0f);

		reserveVertex(vertexCount * cubeFaceCount);
		reserveTriangle(indexCount * cubeFaceCount / 3);

		const Array<Mat33f, cubeFaceCount> transforms = {
			Mat33f::cIdentity,
			Mat33f::rotate(0, Math<float>::cPiOver2() * 1, 0),
			Mat33f::rotate(0, Math<float>::cPiOver2() * 2, 0),
			Mat33f::rotate(0, Math<float>::cPiOver2() * 3, 0),
			Mat33f::rotate(Math<float>::cPiOver2() * 1, 0, 0),
			Mat33f::rotate(Math<float>::cPiOver2() * 3, 0, 0),
		};

		// Loop for the 6 faces of the unit cube
		for(size_t cubeFace = 0; cubeFace < cubeFaceCount; ++cubeFace)
		{
			Vec3f position(0, 0, extend.z);
			Vec3f corner(0, 0, extend.z - filletRadius);

			for(size_t x = 0; x < columnCount; ++x)
			{
				if(x <= filletSegmentCount) {
					corner.x = center.x - extend.x + filletRadius;
					position.x = corner.x - filletRadius + x * segmentWidth;
				}
				else {
					corner.x = center.x + extend.x - filletRadius;
					position.x = corner.x + (x - filletSegmentCount - 1) * segmentWidth;
				}

				for(size_t y = 0; y < rowCount; ++y)
				{
					if(y <= filletSegmentCount) {
						corner.y = center.y - extend.y + filletRadius;
						position.y = corner.y - filletRadius + y * segmentWidth;
					}
					else {
						corner.y = center.y + (extend.y - filletRadius);
						position.y = corner.y + (y - filletSegmentCount - 1) * segmentWidth;
					}

					Vec3f diff = position - corner;
					Vec3f p(corner), n(0, 0, 1);

					if(diff.length() > 0) {
						// Normalize the vector length to form the curve
						p += diff * (filletRadius / (position - corner).length());
						n = diff.normalizedCopy();
					}

					p = transforms[cubeFace] * p;
					n = transforms[cubeFace] * n;

					MeshBuilder::position(p);
					MeshBuilder::normal(n);
					MeshBuilder::addVertex();
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

		end();
	}

	void commit(Mesh& mesh, StorageHint storageHint)
	{
		MeshBuilder::commit(mesh, storageHint);
	}
};	// ChamferBoxBuilder
