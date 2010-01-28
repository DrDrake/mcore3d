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
	/*!	Copy selective vertex data from one builder to another.
		The vertex data in srcBuilder specified by srcIndex will fill to the position in destBuilder specified by destIndex.
		If destIndex is null, the vertex data in srcBuilder specified by srcIndex will fill to destBuilder in sequence.
		It will not modify the index buffer of destBuilder.
		The destBuilder will be resized accordingly.
		Requirement:
		-Both builder should have the same attribute declarations.
		-The index in srcIndex should be within the vertex count of srcBuilder.
		-The index in destIndex should be within the vertex count of destBuilder.
		-The size of srcIndex and destIndex must be the same.
		Limitations:
		-Duplicated value in srcIndex will resulting duplicated vertex data in destBuilder
	 */
	static sal_checkreturn bool copyVertexAttributes(
		MeshBuilder2& srcBuilder, MeshBuilder2& destBuilder,
		FixStrideArray<uint16_t> srcIndex, FixStrideArray<uint16_t> destIndex=FixStrideArray<uint16_t>(nullptr,0)
	);

	static void split(size_t splitCount, MeshBuilder2& srcBuilder, MeshBuilder2** outBuilders, StrideArray<uint16_t>* faceIndices);

	/*!	Compute vertex normals
		Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
		Reference: http://www.devmaster.net/forums/showthread.php?t=414
	 */
	static void computNormal(MeshBuilder2& builder, size_t whichBufferIdStoreNormal);
};	// MeshBuilderUtility

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDERUTILITY__
