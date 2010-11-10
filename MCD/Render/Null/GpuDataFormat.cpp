#include "Pch.h"
#include "../GpuDataFormat.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/Utility.h"

namespace MCD {

GpuDataFormat GpuDataFormat::get(const StringHash& name)
{
	static const GpuDataFormat cGpuDataFormatMap[] = {
	{ FixString("none"), -1, -1, -1, 0, 0, false },

	// 3 components
	{ FixString("uintRGB8"), 0, 0, 0, sizeof(uint8_t), 3, false },
	{ FixString("uintBGR8"), 0, 0, 0, sizeof(uint8_t), 3, false },
	{ FixString("uintB5G6R5"), 0, 0, 0, 0, 3, false },

	// 4 components
	{ FixString("uintRGBA8"), 0, 0, 0, sizeof(uint8_t), 4, false },
	{ FixString("uintBGRA8"), 0, 0, 0, sizeof(uint8_t), 4, false },
	{ FixString("uintBGR5A1"), 0, 0, 0, 0, 4, false },

	// Depth
	{ FixString("depth16"), 0, 0, 0, sizeof(uint16_t), 1, false },
	{ FixString("depth24"), 0, 0, 0, 3 * sizeof(uint8_t), 1, false },
	{ FixString("depth32"), 0, 0, 0, sizeof(float), 1, false },

	// Compressed
	{ FixString("dxt1"), 0, -1, -1, 0, 0, true },
	{ FixString("dxt3"), 0, -1, -1, 0, 0, true },
	{ FixString("dxt5"), 0, -1, -1, 0, 0, true },
	};

	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=0; i<MCD_COUNTOF(cGpuDataFormatMap); ++i) {
		if(cGpuDataFormatMap[i].name.hashValue() == name)
			return cGpuDataFormatMap[i];
	}
	return cGpuDataFormatMap[0];
}

GpuDataFormat GpuDataFormat::none() { return GpuDataFormat::get(StringHash("nonoe")); }

}	// namespace MCD
