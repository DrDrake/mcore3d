#include "Pch.h"
#include "VertexFormat.h"
#include "../Core/System/Utility.h"

namespace MCD {

static const VertexFormat cMap[] = {
	{ FixString("none"),		GpuDataFormat::get("none"), 0 },				// -1
	{ FixString("index"),		GpuDataFormat::get("uintR16"), 0 },				// 0
	{ FixString("position"),	GpuDataFormat::get("floatRGB32"), 0 },
	{ FixString("normal"),		GpuDataFormat::get("floatRGB32"), 0 },
	{ FixString("tangent"),		GpuDataFormat::get("floatRGB32"), 0 },
	{ FixString("binormal"),	GpuDataFormat::get("floatRGB32"), 0 },
	{ FixString("jointWeight"),	GpuDataFormat::get("floatRGBA32"), 0 },
	{ FixString("jointIndex"),	GpuDataFormat::get("uintRGBA8"), 0 },
	{ FixString("uv0"),			GpuDataFormat::get("floatRG32"), 0 },			// 7
	{ FixString("uv1"),			GpuDataFormat::get("floatRG32"), 1 },
	{ FixString("uv2"),			GpuDataFormat::get("floatRG32"), 2 },
	{ FixString("uv3"),			GpuDataFormat::get("floatRG32"), 3 },
	{ FixString("color0"),		GpuDataFormat::get("none"), 0 },				// 11
	{ FixString("color1"),		GpuDataFormat::get("none"), 1 },
	{ FixString("color2"),		GpuDataFormat::get("none"), 2 },
	{ FixString("color3"),		GpuDataFormat::get("none"), 3 },
};

VertexFormat VertexFormat::get(const StringHash& stringHash)
{
	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=1; i<MCD_COUNTOF(cMap); ++i) {
		if(cMap[i].semantic.hashValue() == stringHash)
			return cMap[i];
	}
	MCD_ASSERT(false);
	return cMap[0];
}

VertexFormat VertexFormat::none()
{
	return cMap[0];
}

}	// namespace MCD
