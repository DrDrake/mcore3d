#ifndef __MCD_RENDER_TANGENTSPACEBUILDER__
#define __MCD_RENDER_TANGENTSPACEBUILDER__

#include "ShareLib.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/Array.h"

namespace MCD {

class MeshBuilder;

/*!	Computes tangent space for MeshBuilders.
	Usage:
	\code
	TangentSpaceBuilder tsbuilder;
	MeshBuilder meshBuilder;

	int posIdx = meshBuilder.declareAttribute(SemanticMap::getSingleton().position(), 1);
	int normalIdx = meshBuilder.declareAttribute(SemanticMap::getSingleton().normal(), 1);
	int uvIdx = meshBuilder.declareAttribute(SemanticMap::getSingleton().uv(0, 2), 1);
	int tangentIdx = meshBuilder.declareAttribute(SemanticMap::getSingleton().tangent(), 1);

	tsbuilder.compute(meshBuilder, 0, posIdx, normalIdx, uvIdx, tangentIdx);
	\endcode
 */
class MCD_RENDER_API TangentSpaceBuilder
{
public:
	//!	Returns false if any of the attribute index is invalid.
	sal_checkreturn bool compute(
		MeshBuilder& builder, int indexIdx,
		int posIdx, int normalIdx,
		int uvIdx, int tangentIdx
	);

	void compute(
		const StrideArray<uint16_t>& indexArray,
		const StrideArray<Vec3f>& positionArray,
		const StrideArray<Vec3f>& normalArray,
		const StrideArray<Vec2f>& uvArray,
		StrideArray<Vec3f>& outTangentArray
	);
};	// TangentSpaceBuilder

}	// namespace MCD

#endif	// __MCD_RENDER_TANGENTSPACEBUILDER__