#include "Pch.h"
#include "../GpuDataFormat.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/Utility.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

// NOTE: if you have updated the following table, remember to update the comments in GpuDataFormat!!
static const GpuDataFormat cGpuDataFormatMap[] = {
	{ FixString("none"), -1, -1, -1, 0, 0 },

	// 3 components
	{ FixString("uintR8G8B8"), GL_RGB8, GL_UNSIGNED_BYTE, GL_RGB, sizeof(uint8_t), 3 },

	// 4 components
	{ FixString("uintR8G8B8A8"), GL_RGBA8, GL_UNSIGNED_BYTE, GL_RGBA, sizeof(uint8_t), 4 },

	// Depth
	{ FixString("depth16"), GL_DEPTH_COMPONENT16, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, sizeof(uint16_t), 1 },
	{ FixString("depth24"), GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, 3 * sizeof(uint8_t), 1 },
//	{ FixString("depth24"), GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT, GL_DEPTH_STENCIL, 3 * sizeof(uint8_t), 1 },
	{ FixString("depth32"), GL_DEPTH_COMPONENT32F, GL_FLOAT, GL_DEPTH_COMPONENT, sizeof(float), 1 },
};

GpuDataFormat GpuDataFormat::get(const StringHash& name)
{
	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=0; i<MCD_COUNTOF(cGpuDataFormatMap); ++i) {
		if(cGpuDataFormatMap[i].name.hashValue() == name)
			return cGpuDataFormatMap[i];
	}
	return cGpuDataFormatMap[0];
}

}	// namespace MCD

//#include "../VertexFormat.inc"
