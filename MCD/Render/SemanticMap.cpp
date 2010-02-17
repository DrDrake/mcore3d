#include "Pch.h"
#include "SemanticMap.h"
#include "../Core/System/Utility.h"	// for MCD_COUNTOF
#include <memory>	// for auto_ptr

namespace MCD {

SemanticMap::SemanticMap() {}

static const size_t cMaxUvChannel = 8;
static const size_t cMaxColorChannel = 4;

static const SemanticMap::Semantic cMap[19] = {
	{ "index",		MeshBuilder::TYPE_UINT16, sizeof(uint16_t), 1, 0 },	// 0
	{ "position",	MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ "normal",		MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ "tangent",	MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ "binormal",	MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 },
	{ "blendWeight",MeshBuilder::TYPE_FLOAT, sizeof(float), 4, 0 },
	{ "blendIndex",	MeshBuilder::TYPE_UINT8, sizeof(uint8_t), 4, 0 },
	{ "uv0",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 0 },		// 7
	{ "uv1",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 1 },
	{ "uv2",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 2 },
	{ "uv3",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 3 },
	{ "uv4",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 4 },
	{ "uv5",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 5 },
	{ "uv6",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 6 },
	{ "uv7",		MeshBuilder::TYPE_FLOAT, sizeof(float), 0, 7 },
	{ "color0",		MeshBuilder::TYPE_NONE, 0, 0, 0 },					// 15
	{ "color1",		MeshBuilder::TYPE_NONE, 0, 0, 1 },
	{ "color2",		MeshBuilder::TYPE_NONE, 0, 0, 2 },
	{ "color3",		MeshBuilder::TYPE_NONE, 0, 0, 3 },
};

SemanticMap::Semantic SemanticMap::index() const		{ return cMap[0]; }
SemanticMap::Semantic SemanticMap::position() const		{ return cMap[1]; }
SemanticMap::Semantic SemanticMap::normal() const		{ return cMap[2]; }
SemanticMap::Semantic SemanticMap::tangent() const		{ return cMap[3]; }
SemanticMap::Semantic SemanticMap::binormal() const		{ return cMap[4]; }
SemanticMap::Semantic SemanticMap::blendWeight() const	{ return cMap[5]; }
SemanticMap::Semantic SemanticMap::blendIndex() const	{ return cMap[6]; }

SemanticMap::Semantic SemanticMap::uv(size_t index, size_t elementCount) const
{
	Semantic map = cMap[7 + index];
	map.elementCount = elementCount;
	if(!(index < cMaxUvChannel && elementCount > 0 && elementCount <= 4)) map.name = "";
	return map;
}

SemanticMap::Semantic SemanticMap::color(size_t index, size_t elementCount, size_t elementSize) const
{
	Semantic map = cMap[15 + index];
	map.elementCount = elementCount;
	map.elementSize = elementSize;
	map.elementType = elementSize == 1 ? MeshBuilder::TYPE_UINT8 : MeshBuilder::TYPE_FLOAT;
	MCD_ASSUME(elementSize == 1 || elementSize == 4);
	if(!(index < cMaxColorChannel && elementCount > 0 && elementCount <= 4)) map.name = "";
	return map;
}

bool SemanticMap::find(const char* name, SemanticMap::Semantic& semantic)
{
	MCD_ASSUME(name);
	for(size_t i=0; i<MCD_COUNTOF(cMap); ++i) {
		if(strcmp(name, cMap[i].name) == 0) {
			semantic = cMap[i];
			return true;
		}
	}
	return false;
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
