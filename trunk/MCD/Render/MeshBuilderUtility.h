#ifndef __MCD_RENDER_MESHBUILDERUTILITY__
#define __MCD_RENDER_MESHBUILDERUTILITY__

#include "ShareLib.h"
#include "../Core/System/Array.h"

namespace MCD {

class MeshBuilder2;
class MeshBuilderIM;

/*!
 */
class MCD_RENDER_API MeshBuilderUtility
{
public:
	/*!	Copy vertex data from one builder to another.
		The index buffer of the destBuilder will simply copy from desdtIndex.
		Requirement:
		-Both builder should have the same attribute declarations.
		-The destBuilder should resized with enough vertex count.
		-Both index array should have the same size.
		-The index in both index array should be within the vertex count of their corresponding builder.
		-The vertex count in srcBuilder can be equals or larger than that of destBuilder.
	 */
	static bool copyVertexAttributes(
		MeshBuilder2& srcBuilder, MeshBuilder2& destBuilder,
		FixStrideArray<uint16_t> srcIndex, FixStrideArray<uint16_t> destIndex
	);

	static void split(size_t splitCount, MeshBuilder2& srcBuilder, MeshBuilderIM* outBuilders, StrideArray<uint16_t>* faceIndices);

	/*!	Compute vertex normals
		Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
		Reference: http://www.devmaster.net/forums/showthread.php?t=414
	 */
	static void computNormal(MeshBuilder2& builder, size_t whichBufferIdStoreNormal);
};	// MeshBuilderUtility

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDERUTILITY__
