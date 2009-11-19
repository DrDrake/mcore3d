#ifndef __MCD_RENDER_TANGENTSPACEBUILDER__
#define __MCD_RENDER_TANGENTSPACEBUILDER__

#include "ShareLib.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

class MeshBuilder;
class MeshBuilder2;

/*!	The compute tangent space for MeshBuilders.

	Usage:
	\code
	TangentSpaceBuilder tsbuilder;

	Mesh::DataType sourceUV = Mesh::TextureCoord0;
	Mesh::DataType outputUV = Mesh::TextureCoord1;

	// please make sure meshBuilder have allocate the space for
	// outputUV (as an array of Vec3)
	tsbuilder.compute(meshBuilder, sourceUV, outputUV);
	\endcode
 */
class MCD_RENDER_API TangentSpaceBuilder
{
public:
	sal_checkreturn bool compute(MeshBuilder& builder, int uvDataType, int tangentDataType);

	//!	Returns false if any of the attribute index is invalid.
	sal_checkreturn bool compute(
		MeshBuilder2& builder, int indexIdx,
		int posIdx, int normalIdx,
		int uvIdx, int tangentIdx
	);

	void compute(
		const size_t faceCnt, const size_t vertexCnt,
		sal_in_ecount(faceCnt * 3) const uint16_t* indexBuf,
		sal_in_ecount(vertexCnt) const Vec3f* posBuf,
		sal_in_ecount(vertexCnt) const Vec3f* nrmBuf,
		sal_in_ecount(vertexCnt) const Vec2f* uvBuf,
		sal_out_ecount(vertexCnt) Vec3f* outTangBuf
	);
};	// TangentSpaceBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_TANGENTSPACEBUILDER__