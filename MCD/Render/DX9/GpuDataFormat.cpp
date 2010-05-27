#include "Pch.h"
#include "../GpuDataFormat.h"
#include "../../Core/System/StrUtility.h"
#include "../../Core/System/Utility.h"

#include "../../Core/System/PlatformInclude.h"
#include <d3d9types.h>

namespace MCD {

// NOTE: if you have updated the following table, remember to update the comments in GpuDataFormat!!
static const GpuDataFormat cGpuDataFormatMap[] = {
	{ FixString("none"), -1, -1, -1, 0, 0 },

	// 3 components
	{ FixString("uintR8G8B8"), D3DFMT_R8G8B8, D3DFMT_R8G8B8, D3DFMT_R8G8B8, sizeof(uint8_t), 3 },

	// 4 components
	{ FixString("uintR8G8B8A8"), D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, sizeof(uint8_t), 4 },
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
