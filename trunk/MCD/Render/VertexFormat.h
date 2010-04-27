#ifndef __MCD_RENDER_VERTEXFORMAT__
#define __MCD_RENDER_VERTEXFORMAT__

#include "ShareLib.h"
#include "../Core/System/StringHash.h"

namespace MCD {

/*!	This class encapsulate an API independent vertex data format.
 */
struct MCD_RENDER_API VertexFormat
{
	enum ComponentType
	{
		TYPE_NOT_USED = 0,
		TYPE_INT,		TYPE_UINT,
		TYPE_INT8,		TYPE_UINT8,
		TYPE_INT16,		TYPE_UINT16,
		TYPE_FLOAT,		TYPE_DOUBLE,
		TYPE_NONE	//!< For error indication
	};	// ComponentType

	//! Name for this format e.g. index, position, normal, uv0, uv1...
	FixString semantic;

	ComponentType componentType;

	//! Size in byte of the attribute, ie: Vec3f -> sizeof(float)
	uint8_t componentSize;

	//! Number of components, ie: Scalar = 1; Vec2f = 2; Vec3f = 3; Vec4f = 4;
	uint8_t componentCount;

	uint8_t channel;

	//! The size in byte of this vertex data format
	size_t sizeInByte() const {
		return componentCount * componentSize;
	}

	//!	Convert to API dependent type, ie: GL_FLOAT, GL_UNSIGNED_BYTE
	static int toApiDependentType(ComponentType type);
};	// VertexFormat

//!	The default location that stores a list of pre-defined VertexFormat.
class MCD_RENDER_API VertexFormatFactory
{
public:
	/*!	Get a VertexFormat by a semantic name.
		There are pre-defined VretexFormat implemented in the API specific VertexFormat.cpp
		Returns a dummy VertexFormat if semantic cannot found, and gives assertion on debug mode.
	 */
	static VertexFormat get(const StringHash& semantic);
};	// VertexFormatFactory

}	// namespace MCD

#endif	// __MCD_RENDER_VERTEXFORMAT__
