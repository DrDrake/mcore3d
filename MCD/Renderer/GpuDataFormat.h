#ifndef __MCD_RENDERER_GPUDATAFORMAT__
#define __MCD_RENDERER_GPUDATAFORMAT__

#include "ShareLib.h"
#include "../Core/System/StringHash.h"

namespace MCD {

/*! This class encapsulate an API independent Gpu data format.
	Gpu data include:
	- Texture
	- RenderTarget
	- Mesh
	- ShaderConstantBuffer

	TODO: make all Gpu data use this class for creation.
 */
class MCD_RENDERER_API GpuDataFormat
{
public:
	explicit GpuDataFormat(const StringHash& stringHash);

	//! Name for this format e.g. floatR16G16B16A16, unormR8G8B8A8
	const FixString name;

	/*!	API dependent format enum
		e.g.
		- GL_RGBA16F_ARB
		- DXGI_FORMAT_R16G16B16A16_FLOAT
	 */
	int format;

	/*!	API dependent data type enum e.g. GL_FLOAT, GL_UNSIGNED_BYTE
		Not all API use this field.
	 */
	int dataType;

	/*!	API dependent components enum e.g. GL_RGBA, GL_LUMINANCE
		Not all API use this field.
	 */
	int components;

	//! Size in byte of the attribute, ie: Vec3f -> sizeof(float)
	uint8_t componentSize;

	//! Number of components, ie: Scalar = 1; Vec2f = 2; Vec3f = 3; Vec4f = 4;
	uint8_t componentCount;

	//! The size in byte of this gpu data format
	size_t sizeInByte() const {
		return componentCount * componentSize;
	}

	bool operator == (const GpuDataFormat& rvalue) const {
		return name == rvalue.name;
	}

	bool operator != (const GpuDataFormat& rvalue) const {
		return !(*this == rvalue);
	}
};	// GpuDataFormat

}	// namespace MCD

#endif	// __MCD_RENDERER_GPUDATAFORMAT__
