#ifndef __MCD_RENDER_GPUDATAFORMAT__
#define __MCD_RENDER_GPUDATAFORMAT__

#include "ShareLib.h"
#include "../Core/System/StringHash.h"

namespace MCD {

/*!	Encapsulates graphics API dependent Gpu data formats.
	Applys to:
	- Texture
 */
struct MCD_RENDER_API GpuDataFormat
{
public:
	//! Name for this format e.g. uintRGB8, unormRGBA, for debug purpose
	FixString name;

	/*!	API dependent format enum
		e.g.
		- GL_RGBA16F_ARB
		- D3DFMT_A8R8G8B8
	 */
	int format;

	/*! API dependent data type enum e.g. GL_FLOAT, GL_UNSIGNED_BYTE
		Not all API use this field.
	 */
	int dataType;

	/*! API dependent components enum e.g. GL_RGBA, GL_LUMINANCE
		Not all API use this field.
	 */
	int components;

	/*!	Size in byte of the attribute, ie: Vec3f -> sizeof(float)
		Zero if it is not applicatable.
	 */
	uint8_t componentSize;

	//! Number of components, ie: Scalar = 1; Vec2f = 2; Vec3f = 3; Vec4f = 4;
	uint8_t componentCount;

	bool isCompressed;

	//! The size in byte of this gpu data format
	size_t sizeInByte() const { return componentCount * componentSize; }

	sal_checkreturn bool isValid() const { return format != -1; }

	bool operator==(const GpuDataFormat& rhs) const { return format == rhs.format; }

	bool operator!=(const GpuDataFormat& rhs) const { return !(*this == rhs); }

	static GpuDataFormat none();

	/*! Parse a GpuDataFormat object from string.
		If the requesting format is not supported in the platform, 'none' is returned
	 */
	static GpuDataFormat get(const StringHash& name);
};	// GpuDataFormat

}	// namespace MCD

#endif	// __MCD_RENDER_GPUDATAFORMAT__
