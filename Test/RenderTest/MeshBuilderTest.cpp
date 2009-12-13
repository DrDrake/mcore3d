#include "Pch.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Render/MeshBuilderUtility.h"
#include "../../MCD/Core/System/Array.h"

using namespace MCD;

namespace {

const MeshBuilder2::Semantic indexSemantic =		{ "index", sizeof(uint16_t), 1, 0 };
const MeshBuilder2::Semantic positionSemantic =		{ "position", sizeof(float), 3, 0 };
const MeshBuilder2::Semantic normalSemantic =		{ "normal", sizeof(float), 3, 0 };
const MeshBuilder2::Semantic uvSemantic =			{ "uv", sizeof(float), 2, 0 };
const MeshBuilder2::Semantic blendWeightSemantic =	{ "blendWeight", sizeof(float), 1, 0 };

}	// namespace

TEST(copyVertexAttributes_MeshBuilderUtilityTest)
{
	{	// Simple create and destroy a mesh builder
		MeshBuilder2 srcBuilder, destBuilder;
		CHECK(MeshBuilderUtility::copyVertexAttributes(
			srcBuilder, destBuilder, FixStrideArray<uint16_t>(nullptr, 0), FixStrideArray<uint16_t>(nullptr, 0)
		));
	}

	{	// Check against the initial states
		MeshBuilder2 srcBuilder, destBuilder;

		int posId = srcBuilder.declareAttribute(positionSemantic, 1);
		int normalId = srcBuilder.declareAttribute(normalSemantic, 1);
		int uvId = srcBuilder.declareAttribute(uvSemantic, 2);	// Note that we use a separate buffer for uv

		CHECK(srcBuilder.resizeBuffers(8, 12));

		StrideArray<Vec3f> pos = srcBuilder.getAttributeAs<Vec3f>(posId);
		StrideArray<Vec3f> normal = srcBuilder.getAttributeAs<Vec3f>(normalId);
		StrideArray<Vec2f> uv = srcBuilder.getAttributeAs<Vec2f>(uvId);

		for(size_t i=0; i<srcBuilder.vertexCount(); ++i) {
			pos[i] = Vec3f(float(i));
			normal[i] = Vec3f(float(i) * 2);
			uv[i] = Vec2f(float(i) * 3);
		}

		StrideArray<uint16_t> idx = srcBuilder.getAttributeAs<uint16_t>(0);
		for(uint16_t i=0; i<srcBuilder.indexCount()/3; ++i) {
			idx[i*3+0] = i+0;
			idx[i*3+1] = i+1;
			idx[i*3+2] = i+2;
		}

		CHECK(MeshBuilderUtility::copyVertexAttributes(
			srcBuilder, destBuilder, FixStrideArray<uint16_t>(nullptr, 0), FixStrideArray<uint16_t>(nullptr, 0)
		));
	}
}
