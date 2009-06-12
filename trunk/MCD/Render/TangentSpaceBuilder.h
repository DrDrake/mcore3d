#ifndef __MCD_RENDER_TANGENTSPACEBUILDER__
#define __MCD_RENDER_TANGENTSPACEBUILDER__

#include "ShareLib.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"

namespace MCD
{

class MeshBuilder;

/*!	The compute tangent space for MeshBuilders.

	Usage:
	\code
	TangentSpaceBuilder tsbuilder;

	Mesh::DataType sourceUV = Mesh::TextureCoord0;
	Mesh::DataType outputUV = Mesh::TextureCoord1;

	// please make sure meshBuilder have allocate the space for
	// outputUV (as an array of Vec3)
	tsbuilder.compute( meshBuilder, sourceUV, outputUV );
	\endcode
 */
class MCD_RENDER_API TangentSpaceBuilder
{
public:
	void compute( MeshBuilder& builder, int uvDataType, int tangentDataType );

	void compute(
		const size_t faceCnt, const size_t vertexCnt,
		sal_in_ecount(faceCnt * 3) const uint16_t* indexBuf,
		sal_in_ecount(vertexCnt) const Vec3f* posBuf,
		sal_in_ecount(vertexCnt) const Vec3f* nrmBuf,
		sal_in_ecount(vertexCnt) const Vec2f* uvBuf,
		sal_out_ecount(vertexCnt) Vec3f* outTangBuf
	);

protected:
	static void computeTangentBasis(
		const Vec3f& P1, const Vec3f& P2, const Vec3f& P3, 
		const Vec2f& UV1, const Vec2f& UV2, const Vec2f& UV3,
		Vec3f& tangent, Vec3f& bitangent
	);
};

}	// namespace MCD

#endif	// __MCD_RENDER_TANGENTSPACEBUILDER__