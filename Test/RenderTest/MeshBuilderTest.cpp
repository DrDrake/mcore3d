#include "Pch.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Render/MeshBuilderUtility.h"
#include "../../MCD/Core/System/Array.h"
#include "../../MCD/Core/System/Utility.h"

using namespace MCD;

namespace {

const MeshBuilder2::Semantic indexSemantic =		{ "index", sizeof(uint16_t), 1, 0 };
const MeshBuilder2::Semantic positionSemantic =		{ "position", sizeof(float), 3, 0 };
const MeshBuilder2::Semantic normalSemantic =		{ "normal", sizeof(float), 3, 0 };
const MeshBuilder2::Semantic uvSemantic =			{ "uv", sizeof(float), 2, 0 };
const MeshBuilder2::Semantic blendWeightSemantic =	{ "blendWeight", sizeof(float), 1, 0 };

class MeshBuilderTestFixture
{
protected:
	MeshBuilderTestFixture()
		: pos(nullptr, 0), normal(nullptr, 0), uv(nullptr, 0), idx(nullptr, 0)
	{
		posId = srcBuilder.declareAttribute(positionSemantic, 1);
		normalId = srcBuilder.declareAttribute(normalSemantic, 1);
		uvId = srcBuilder.declareAttribute(uvSemantic, 2);	// Note that we use a separate buffer for uv
	}

	bool setupSrcBuilder()
	{
		if(!srcBuilder.resizeBuffers(8, 12))
			return false;

		pos = srcBuilder.getAttributeAs<Vec3f>(posId);
		normal = srcBuilder.getAttributeAs<Vec3f>(normalId);
		uv = srcBuilder.getAttributeAs<Vec2f>(uvId);

		for(size_t i=0; i<srcBuilder.vertexCount(); ++i) {
			pos[i] = Vec3f(float(i));
			normal[i] = Vec3f(float(i) * 2);
			uv[i] = Vec2f(float(i) * 3);
		}

		idx = srcBuilder.getAttributeAs<uint16_t>(0);
		for(uint16_t i=0; i<srcBuilder.indexCount()/3; ++i) {
			idx[i*3+0] = i+0;
			idx[i*3+1] = i+1;
			idx[i*3+2] = i+2;
		}

		return true;
	}

	MeshBuilder2 srcBuilder;
	int posId, normalId, uvId;
	StrideArray<Vec3f> pos, normal;
	StrideArray<Vec2f> uv;
	StrideArray<uint16_t> idx;
};	// MeshBuilderTestFixture

}	// namespace

TEST(copyEmptyVertexAttributes_MeshBuilderUtilityTest)
{
	// Empty builder
	MeshBuilder2 srcBuilder, destBuilder;
	CHECK(MeshBuilderUtility::copyVertexAttributes(
		srcBuilder, destBuilder, FixStrideArray<uint16_t>(nullptr, 0)
	));
}

TEST_FIXTURE(MeshBuilderTestFixture, copyVertexAttributes)
{
	CHECK(setupSrcBuilder());

	// Setting up destBuilder
	MeshBuilder2 destBuilder;
	destBuilder.declareAttribute(positionSemantic, 1);
	destBuilder.declareAttribute(normalSemantic, 1);
	destBuilder.declareAttribute(uvSemantic, 2);

	uint16_t srcIndex[] = { 4, 1, 1, 3 };	// Note that we have tested many things in this srcIndex
	CHECK(MeshBuilderUtility::copyVertexAttributes(
		srcBuilder, destBuilder, FixStrideArray<uint16_t>(srcIndex, MCD_COUNTOF(srcIndex))
	));

	CHECK_EQUAL(srcBuilder.attributeCount(), destBuilder.attributeCount());
	CHECK_EQUAL(destBuilder.vertexCount(), MCD_COUNTOF(srcIndex));
	CHECK_EQUAL(0u, destBuilder.indexCount());	// The index buffer of destBuilder didn't changed

	StrideArray<Vec3f> pos_ = destBuilder.getAttributeAs<Vec3f>(posId);
	StrideArray<Vec3f> normal_ = destBuilder.getAttributeAs<Vec3f>(normalId);
	StrideArray<Vec2f> uv_ = destBuilder.getAttributeAs<Vec2f>(uvId);

	for(size_t i=0; i<destBuilder.vertexCount(); ++i) {
		CHECK(pos[srcIndex[i]] == pos_[i]);
		CHECK(normal[srcIndex[i]] == normal_[i]);
		CHECK(uv[srcIndex[i]] == uv_[i]);
	}
}

TEST_FIXTURE(MeshBuilderTestFixture, split)
{
	CHECK(setupSrcBuilder());

	static const size_t cSplitCount = 3;
	MeshBuilder2 destBuilders[cSplitCount];

	for(size_t i=0; i<MCD_COUNTOF(destBuilders); ++i) {
		destBuilders[i].declareAttribute(positionSemantic, 1);
		destBuilders[i].declareAttribute(normalSemantic, 1);
		destBuilders[i].declareAttribute(uvSemantic, 2);
	}

	uint16_t i_[cSplitCount][4] =			{ { 4, 7, 7, 3 }, { 5, 6, 7, 999 }, { 2, 2, 5, 2 } };
	uint16_t expectedIdx_[cSplitCount][4] =	{ { 0, 1, 1, 2 }, { 0, 1, 2, 999 }, { 0, 0, 1, 0 } };

	StrideArray<uint16_t> indices[cSplitCount] = {
		StrideArray<uint16_t>(i_[0], 4), StrideArray<uint16_t>(i_[1], 3), StrideArray<uint16_t>(i_[2], 4)
	};

	MeshBuilderUtility::split(cSplitCount, srcBuilder, destBuilders, indices);

	for(size_t i=0; i<cSplitCount; ++i) {
		MeshBuilder2& destBuilder = destBuilders[i];

		StrideArray<Vec3f> pos_ = destBuilder.getAttributeAs<Vec3f>(posId);
		StrideArray<Vec3f> normal_ = destBuilder.getAttributeAs<Vec3f>(normalId);
		StrideArray<Vec2f> uv_ = destBuilder.getAttributeAs<Vec2f>(uvId);
		StrideArray<uint16_t> idx_ = destBuilder.getAttributeAs<uint16_t>(0);


		for(size_t j=0; j<idx_.size; ++j) {
			CHECK_EQUAL(expectedIdx_[i][j], idx_[j]);
			CHECK(pos[i_[i][j]] == pos_[idx_[j]]);
			CHECK(normal[i_[i][j]] == normal_[idx_[j]]);
			CHECK(uv[i_[i][j]] == uv_[idx_[j]]);
		}
	}
}
