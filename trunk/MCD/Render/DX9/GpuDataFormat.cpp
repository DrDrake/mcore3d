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

	// 1 component
	{ FixString("uintR16"), -1, -1, -1, sizeof(uint16_t), 1, false },

	// 2 components
	{ FixString("floatRG32"), -1, D3DDECLTYPE_FLOAT2, -1, sizeof(float), 2, false },

	// 3 components
	{ FixString("uintRGB8"), D3DFMT_R8G8B8, D3DFMT_R8G8B8, D3DFMT_R8G8B8, sizeof(uint8_t), 3, false },
	{ FixString("floatRGB32"), -1, D3DDECLTYPE_FLOAT3, -1, sizeof(float), 3, false },

	// 4 components
	{ FixString("uintRGBA8"), -1, D3DDECLTYPE_UBYTE4, -1, sizeof(uint8_t), 4, false },
	{ FixString("uintARGB8"), D3DFMT_A8R8G8B8, D3DDECLTYPE_UBYTE4, D3DFMT_A8R8G8B8, sizeof(uint8_t), 4, false },
	{ FixString("floatRGBA32"), -1, D3DDECLTYPE_FLOAT4, -1, sizeof(float), 4, false },

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

GpuDataFormat GpuDataFormat::none() { return GpuDataFormat::get(StringHash("nonoe")); }

}	// namespace MCD
