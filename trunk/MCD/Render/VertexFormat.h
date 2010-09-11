#ifndef __MCD_RENDER_VERTEXFORMAT__
#define __MCD_RENDER_VERTEXFORMAT__

#include "GpuDataFormat.h"
#include "../Core/System/StringHash.h"

namespace MCD {

/*!	This class encapsulate an API independent vertex data format.
 */
struct MCD_RENDER_API VertexFormat
{
	//! Name for this format e.g. index, position, normal, uv0, uv1...
	FixString semantic;

	GpuDataFormat gpuFormat;

	uint8_t channel;

	//! The size in byte of this vertex data format
	size_t sizeInByte() const {
		return gpuFormat.sizeInByte();//componentCount * componentSize;
	}

	/*!	Get a VertexFormat by a semantic name.
		There are pre-defined VretexFormat implemented in the API specific VertexFormat.cpp
		Returns a dummy VertexFormat if semantic cannot found, and gives assertion on debug mode.
	 */
	static VertexFormat get(const StringHash& semantic);

	static VertexFormat none();
};	// VertexFormat

}	// namespace MCD

#endif	// __MCD_RENDER_VERTEXFORMAT__
