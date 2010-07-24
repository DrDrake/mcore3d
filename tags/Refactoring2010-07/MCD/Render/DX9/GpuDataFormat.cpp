#include "Pch.h"
#include "../GpuDataFormat.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/Utility.h"

#include "../../Core/System/PlatformInclude.h"
#include <d3d9types.h>

namespace MCD {

GpuDataFormat GpuDataFormat::get(const StringHash& name)
{
	static const GpuDataFormat cGpuDataFormatMap[] = {
	{ FixString("none"), -1, -1, -1, 0, 0, false },

	// 3 components
	{ FixString("uintRGB8"), D3DFMT_R8G8B8, D3DFMT_R8G8B8, D3DFMT_R8G8B8, sizeof(uint8_t), 3, false },

	// 4 components
	{ FixString("uintRGBA8"), -1, -1, -1, sizeof(uint8_t), 4, false },
	{ FixString("uintARGB8"), D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, sizeof(uint8_t), 4, false },

	// Compressed
	{ FixString("dxt1"), D3DFMT_DXT1, -1, -1, 0, 0, true },
	{ FixString("dxt3"), D3DFMT_DXT3, -1, -1, 0, 0, true },
	{ FixString("dxt5"), D3DFMT_DXT5, -1, -1, 0, 0, true },
	};

	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=0; i<MCD_COUNTOF(cGpuDataFormatMap); ++i) {
		if(cGpuDataFormatMap[i].name.hashValue() == name)
			return cGpuDataFormatMap[i];
	}
	return cGpuDataFormatMap[0];
}

}	// namespace MCD
