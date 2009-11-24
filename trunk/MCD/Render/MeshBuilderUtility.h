#ifndef __MCD_RENDER_MESHBUILDERUTILITY__
#define __MCD_RENDER_MESHBUILDERUTILITY__

#include "ShareLib.h"
#include <vector>

namespace MCD {

class MeshBuilder2;
class MeshBuilderIM;

/*!
 */
class MCD_RENDER_API MeshBuilderUtility
{
public:
	static void split(size_t splitCount, MeshBuilder2& srcBuilder, MeshBuilderIM* outBuilders, std::vector<uint16_t>* faceIndices);

	/*!	Compute vertex normals
		Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
		Reference: http://www.devmaster.net/forums/showthread.php?t=414
	 */
	static void computNormal(MeshBuilder2& builder, size_t whichBufferIdStoreNormal);
};	// MeshBuilderUtility

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDERUTILITY__
