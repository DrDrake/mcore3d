#include "Pch.h"
#include "VertexFormat.h"
#include "../Core/System/Utility.h"

namespace MCD {

static const VertexFormat cMap[] = {
	{ FixString("none"),		VertexFormat::TYPE_NONE, 0, 0, 0 },					// -1
	{ FixString("index"),		VertexFormat::TYPE_UINT16, sizeof(uint16_t), 1, 0 },// 0
	{ FixString("position"),	VertexFormat::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ FixString("normal"),		VertexFormat::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ FixString("tangent"),		VertexFormat::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ FixString("binormal"),	VertexFormat::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ FixString("jointWeight"),	VertexFormat::TYPE_FLOAT, sizeof(float), 4, 0 },
	{ FixString("jointIndex"),	VertexFormat::TYPE_UINT8, sizeof(uint8_t), 4, 0 },
	{ FixString("uv0"),			VertexFormat::TYPE_FLOAT, sizeof(float), 0, 0 },	// 7
	{ FixString("uv1"),			VertexFormat::TYPE_FLOAT, sizeof(float), 0, 1 },
	{ FixString("uv2"),			VertexFormat::TYPE_FLOAT, sizeof(float), 0, 2 },
	{ FixString("uv3"),			VertexFormat::TYPE_FLOAT, sizeof(float), 0, 3 },
	{ FixString("color0"),		VertexFormat::TYPE_NONE, 0, 0, 0 },					// 11
	{ FixString("color1"),		VertexFormat::TYPE_NONE, 0, 0, 1 },
	{ FixString("color2"),		VertexFormat::TYPE_NONE, 0, 0, 2 },
	{ FixString("color3"),		VertexFormat::TYPE_NONE, 0, 0, 3 },
};

VertexFormat VertexFormatFactory::get(const StringHash& stringHash)
{
	// TODO: May use a hash table, if the linear search introduce too much cache miss.
	for(size_t i=1; i<MCD_COUNTOF(cMap); ++i) {
		if(cMap[i].semantic.hashValue() == stringHash)
			return cMap[i];
	}
	MCD_ASSERT(false);
	return cMap[0];
}

}	// namespace MCD
