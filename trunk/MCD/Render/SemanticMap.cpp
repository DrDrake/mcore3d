#include "Pch.h"
#include "SemanticMap.h"
#include "../Core/System/Utility.h"	// for MCD_COUNTOF
#include <memory>	// for auto_ptr

namespace MCD {

SemanticMap::SemanticMap() {}

SemanticMap::Semantic SemanticMap::index() const		{ static const Semantic map = { "index", sizeof(uint16_t), 1, 0 }; return map; }
SemanticMap::Semantic SemanticMap::position() const		{ static const Semantic map = { "position", sizeof(float), 3, 0 }; return map; }
SemanticMap::Semantic SemanticMap::normal() const		{ static const Semantic map = { "normal", sizeof(float), 3, 0 }; return map; }
SemanticMap::Semantic SemanticMap::tangent() const		{ static const Semantic map = { "tangent", sizeof(float), 3, 0 }; return map; }
SemanticMap::Semantic SemanticMap::binormal() const		{ static const Semantic map = { "binormal", sizeof(float), 3, 0 }; return map; }
SemanticMap::Semantic SemanticMap::blendWeight() const	{ static const Semantic map = { "blendWeight", sizeof(float), 1, 0 }; return map; }
SemanticMap::Semantic SemanticMap::blendIndex() const	{ static const Semantic map = { "blendIndex", sizeof(uint8_t), 1, 0 }; return map; }

SemanticMap::Semantic SemanticMap::uv(size_t index, size_t elementCount) const
{
	static const char* str[] = { "uv0", "uv1", "uv2", "uv3", "uv4", "uv5", "uv6", "uv7" };
	Semantic map = { "", sizeof(float), elementCount, index };
	if(index < MCD_COUNTOF(str) && elementCount > 0 && elementCount <= 4) map.name = str[index];
	return map;
}
SemanticMap::Semantic SemanticMap::color(size_t index, size_t elementCount, size_t elementSize) const
{
	static const char* str[] = { "color0", "color1", "color2", "color3" };
	Semantic map = { "", elementSize, elementCount, index };
	if(index < MCD_COUNTOF(str) && elementCount > 0 && elementCount <= 4) map.name = str[index];
	return map;
}

static std::auto_ptr<SemanticMap> gSemanticMap;

void SemanticMap::setSingleton(SemanticMap& map)
{
	gSemanticMap.reset(&map);
}

SemanticMap& SemanticMap::getSingleton()
{
	if(!gSemanticMap.get())
		setSingleton(*new SemanticMap);
	return *gSemanticMap.get();
}

}	// namespace MCD
