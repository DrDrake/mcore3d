#include "Pch.h"
#include "../GpuDataFormat.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/Utility.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

GpuDataFormat GpuDataFormat::get(const StringHash& name)
{
	static const GpuDataFormat cGpuDataFormatMap[] = {
	{ FixString("none"), -1, -1, -1, 0, 0, false },

	// 3 components
	{ FixString("uintRGB8"), GL_RGB8, GL_UNSIGNED_BYTE, GL_RGB, sizeof(uint8_t), 3, false },
	{ FixString("uintBGR8"), GL_RGB8, GL_UNSIGNED_BYTE, GL_BGR, sizeof(uint8_t), 3, false },
	{ FixString("uintB5G6R5"), GL_RGB5, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, 0, 3, false },

	// 4 components
	{ FixString("uintRGBA8"), GL_RGBA8, GL_UNSIGNED_BYTE, GL_RGBA, sizeof(uint8_t), 4, false },
	{ FixString("uintBGRA8"), GL_RGBA8, GL_UNSIGNED_BYTE, GL_BGRA, sizeof(uint8_t), 4, false },
	{ FixString("uintBGR5A1"), GL_RGB5_A1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_BGRA, 0, 4, false },

	// Depth
	{ FixString("depth16"), GL_DEPTH_COMPONENT16, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, sizeof(uint16_t), 1, false },
	{ FixString("depth24"), GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, 3 * sizeof(uint8_t), 1, false },
//	{ FixString("depth24"), GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT, GL_DEPTH_STENCIL, 3 * sizeof(uint8_t), 1, false },
	{ FixString("depth32"), GL_DEPTH_COMPONENT32F, GL_FLOAT, GL_DEPTH_COMPONENT, sizeof(float), 1, false },

	// Compressed
	{ FixString("dxt1"), GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, -1, -1, 0, 0, true },
	{ FixString("dxt3"), GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, -1, -1, 0, 0, true },
	{ FixString("dxt5"), GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, -1, -1, 0, 0, true },
	};

	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=0; i<MCD_COUNTOF(cGpuDataFormatMap); ++i) {
		if(cGpuDataFormatMap[i].name.hashValue() == name)
			return cGpuDataFormatMap[i];
	}
	return cGpuDataFormatMap[0];
}

}	// namespace MCD
