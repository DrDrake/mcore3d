#include "Pch.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Core/System/Array.h"

using namespace MCD;

namespace {

const MeshBuilder::Semantic indexSemantic =			{ "index", MeshBuilder::TYPE_UINT16, sizeof(uint16_t), 1, 0 };
const MeshBuilder::Semantic positionSemantic =		{ "position", MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 };
const MeshBuilder::Semantic normalSemantic =		{ "normal", MeshBuilder::TYPE_FLOAT, sizeof(float), 3, 0 };
const MeshBuilder::Semantic uvSemantic =			{ "uv", MeshBuilder::TYPE_FLOAT, sizeof(float), 2, 0 };
const MeshBuilder::Semantic blendWeightSemantic =	{ "blendWeight", MeshBuilder::TYPE_FLOAT, sizeof(float), 1, 0 };

}	// namespace

TEST(MeshBuilderTest)
{
	{	// Simple create and destroy a mesh builder
		MeshBuilder builder;
	}

	{	// Check against the initial states
		MeshBuilder builder;

		CHECK_EQUAL(1u, builder.attributeCount());	// The "index" is already reserved.
		CHECK_EQUAL(0u, builder.vertexCount());
		CHECK_EQUAL(0u, builder.indexCount());
		CHECK_EQUAL(1u, builder.bufferCount());		// There is the build-in index buffer

		// Try to declare the default attribute id 0 (which means index) will fail
		CHECK(0 > builder.declareAttribute(indexSemantic, 0));

		CHECK(!builder.getAttributePointer(0));
		CHECK(!builder.getBufferPointer(0));

		CHECK(builder.resizeBuffers(10, 20));

		CHECK_EQUAL(-1, builder.findAttributeId(""));
	}

	{	// More complicated cases
		MeshBuilder builder;

		int posId = builder.declareAttribute(positionSemantic, 1);
		CHECK(posId > 0);
		CHECK_EQUAL(posId, builder.findAttributeId("position"));

		int normalId = builder.declareAttribute(normalSemantic, 1);
		CHECK(normalId > posId);

		int uvId = builder.declareAttribute(uvSemantic, 1);
		CHECK(uvId > normalId);

		int weightId = builder.declareAttribute(blendWeightSemantic, 2);
		CHECK(weightId > uvId);

		CHECK_EQUAL(5u, builder.attributeCount());

		CHECK(builder.resizeBuffers(8, 12));
		CHECK_EQUAL(8u, builder.vertexCount());
		CHECK_EQUAL(12u, builder.indexCount());
		CHECK_EQUAL(3u, builder.bufferCount());

		size_t count, stride, bufferId, offset;
		MeshBuilder::Semantic semantic;

		// Get the data pointer for the various attributes
		char* pos = builder.getAttributePointer(posId, &count, &stride, &bufferId, &offset, &semantic);
		CHECK(pos);
		CHECK_EQUAL(sizeof(float) * 8, stride);
		CHECK_EQUAL(1u, bufferId);
		CHECK_EQUAL(0u, offset);
		CHECK_EQUAL(builder.vertexCount(), count);
		CHECK_EQUAL(3u, semantic.elementCount);
		CHECK_EQUAL(sizeof(float), semantic.elementSize);
		CHECK_EQUAL(std::string("position"), semantic.name);

		char* normal = builder.getAttributePointer(normalId, &count, &stride, &bufferId, &offset, &semantic);
		CHECK_EQUAL(sizeof(float) * 8, stride);
		CHECK_EQUAL(1u, bufferId);
		CHECK_EQUAL(0u, offset);
		CHECK_EQUAL(builder.vertexCount(), count);
		CHECK_EQUAL(3u, semantic.elementCount);
		CHECK_EQUAL(sizeof(float), semantic.elementSize);
		CHECK_EQUAL(std::string("normal"), semantic.name);
		CHECK_EQUAL(normal, pos + sizeof(float) * 3);

		char* uv = builder.getAttributePointer(uvId, &count, &stride, &bufferId, &offset, &semantic);
		CHECK_EQUAL(sizeof(float) * 8, stride);
		CHECK_EQUAL(1u, bufferId);
		CHECK_EQUAL(0u, offset);
		CHECK_EQUAL(builder.vertexCount(), count);
		CHECK_EQUAL(2u, semantic.elementCount);
		CHECK_EQUAL(sizeof(float), semantic.elementSize);
		CHECK_EQUAL(std::string("uv"), semantic.name);
		CHECK_EQUAL(uv, normal + sizeof(float) * 3);

		char* weight = builder.getAttributePointer(weightId, &count, &stride, &bufferId, &offset, &semantic);
		CHECK(weight);
		CHECK_EQUAL(sizeof(float), stride);
		CHECK_EQUAL(2u, bufferId);
		CHECK_EQUAL(0u, offset);
		CHECK_EQUAL(builder.vertexCount(), count);
		CHECK_EQUAL(1u, semantic.elementCount);
		CHECK_EQUAL(sizeof(float), semantic.elementSize);
		CHECK_EQUAL(std::string("blendWeight"), semantic.name);

		// Get the index buffer pointer
		uint16_t* idxPtr = (uint16_t*)builder.getAttributePointer(0, &count, &stride, &bufferId, &offset, &semantic);
		CHECK(idxPtr);
		CHECK_EQUAL(sizeof(uint16_t), stride);
		CHECK_EQUAL(0u, bufferId);
		CHECK_EQUAL(0u, offset);
		CHECK_EQUAL(builder.indexCount(), count);
		CHECK_EQUAL(1u, semantic.elementCount);
		CHECK_EQUAL(sizeof(uint16_t), semantic.elementSize);
		CHECK_EQUAL(std::string("index"), semantic.name);

		// Get the individual buffer's pointer
		size_t totalSize;
		char* buf0 = builder.getBufferPointer(0, &stride, &totalSize);
		CHECK_EQUAL((char*)idxPtr, buf0);
		CHECK_EQUAL(sizeof(uint16_t), stride);
		CHECK_EQUAL(sizeof(uint16_t)*builder.indexCount(), totalSize);

		char* buf1 = builder.getBufferPointer(1, &stride, &totalSize);
		CHECK_EQUAL((char*)pos, buf1);
		CHECK_EQUAL(sizeof(float)*8, stride);
		CHECK_EQUAL(sizeof(float)*8*builder.vertexCount(), totalSize);
	}
}

TEST(MeshBuilderLateDeclareTest)
{
	MeshBuilderIM builder;

	int posId = builder.declareAttribute(positionSemantic, 1);
	int normalId = builder.declareAttribute(normalSemantic, 1);

	// Adds 2 vertex with 2 attribute first
	const Vec3f pos(1, 2, 3);
	CHECK(builder.vertexAttribute(posId, &pos));

	const Vec3f normal(1, 0, 0);
	CHECK(builder.vertexAttribute(normalId, &normal));

	builder.addVertex();
	builder.addVertex();

	// Then add another attribute declaration
	builder.declareAttribute(uvSemantic, 1);

	CHECK(builder.getAttributeAs<Vec3f>(posId)[0].isNearEqual(pos));
	CHECK(builder.getAttributeAs<Vec3f>(normalId)[0].isNearEqual(normal));

	CHECK(builder.getAttributeAs<Vec3f>(posId)[1].isNearEqual(pos));
	CHECK(builder.getAttributeAs<Vec3f>(normalId)[1].isNearEqual(normal));
}

TEST(MeshBuilderIMTest)
{
	{	// Simple create and destroy a mesh builder
		MeshBuilderIM builder;
	}

	{	// Check against the initial states
		MeshBuilderIM builder;

		CHECK_EQUAL(0u, builder.vertexCount());
		CHECK_EQUAL(0u, builder.indexCount());

		CHECK(!builder.getAttributePointer(0));
		CHECK(!builder.getBufferPointer(0));
	}

	{	// More complicated cases
		MeshBuilderIM builder;

		int posId = builder.declareAttribute(positionSemantic, 1);
		CHECK(posId > 0);
		CHECK_EQUAL(posId, builder.findAttributeId("position"));

		int normalId = builder.declareAttribute(normalSemantic, 1);
		CHECK(normalId > posId);

		int uvId = builder.declareAttribute(uvSemantic, 1);
		CHECK(uvId > normalId);

		int weightId = builder.declareAttribute(blendWeightSemantic, 2);
		CHECK(weightId > uvId);

		CHECK_EQUAL(5u, builder.attributeCount());

		{	uint16_t index[3];
			for(size_t i=0; i<3; ++i) {
				const Vec3f pos(1.23f + i), normal(Vec3f::c001);
				const Vec2f uv(0.0f + i);
				const float weight = float(i);

				CHECK(builder.vertexAttribute(posId, &pos));
				CHECK(builder.vertexAttribute(normalId, &normal));
				CHECK(builder.vertexAttribute(uvId, &uv));
				CHECK(builder.vertexAttribute(weightId, &weight));

				index[i] = builder.addVertex();
				CHECK_EQUAL(i, index[i]);
			}

			CHECK(builder.addTriangle(index[0], index[1], index[2]));
		}

		CHECK_EQUAL(3u, builder.vertexCount());
		CHECK_EQUAL(3u, builder.indexCount());

		// Verify the result
		for(size_t i=0; i<3; ++i) {
			const Vec3f pos(1.23f + i);
			size_t stride;
			char* posPtr = builder.getAttributePointer(posId, nullptr, &stride);
			StrideArray<Vec3f> a(posPtr, builder.vertexCount(), stride);
			CHECK(a[i].isNearEqual(pos));
		}
	}
}
/*
TEST(Basic_MeshBuilderTest)
{
	{	// Simple create and destroy a mesh
		MCD::ResourcePtr mesh = new Mesh(L"");
		mesh = nullptr;
	}

	{	// Simple create and destroy a mesh builder
		MeshBuilder builder;
	}

	{	// Enable with different formats
		MeshBuilder builder;

		builder.enable(Mesh::Position);
		CHECK(builder.format() & Mesh::Position);

		builder.enable(Mesh::Normal);
		CHECK(builder.format() & Mesh::Normal);

		builder.enable(Mesh::Color);
		CHECK(builder.format() & Mesh::Color);

		builder.enable(Mesh::Index);
		CHECK(builder.format() & Mesh::Index);

		for(size_t i=0; i<Mesh::cMaxTextureCoordCount; ++i) {
			builder.enable(Mesh::TextureCoord0 + i);
			Mesh::DataType textureUnit = Mesh::DataType(Mesh::TextureCoord0 + i);
			builder.textureUnit(textureUnit);
			builder.textureCoordSize(2);
			CHECK_EQUAL(uint(textureUnit), (builder.format() & Mesh::TextureCoord));
		}
	}

	{	// Fill some data
		MeshBuilder builder;

		// Mesh::TextureCoord1 also implies Mesh::TextureCoord0
		builder.enable(Mesh::Position | Mesh::Color | Mesh::Normal | Mesh::Index | Mesh::TextureCoord1);

		CHECK(builder.format() & Mesh::Position);
		CHECK(builder.format() & Mesh::Color);
		CHECK(builder.format() & Mesh::Normal);
		CHECK(builder.format() & Mesh::Index);
		CHECK_EQUAL(2u, builder.format() & Mesh::TextureCoord);

		builder.textureUnit(Mesh::TextureCoord0);
		builder.textureCoordSize(2);
		builder.textureUnit(Mesh::TextureCoord1);
		builder.textureCoordSize(3);

		builder.reserveVertex(3);
		builder.reserveTriangle(1);

		builder.color(ColorRGB8::red);
		builder.normal(Vec3f::c001);

		{	uint16_t index[3];
			for(size_t i=0; i<3; ++i) {
				builder.position(Vec3f(1.23f + i));
				builder.textureUnit(Mesh::TextureCoord0);
				builder.textureCoord(Vec2f::c01 + float(i));
				builder.textureUnit(Mesh::TextureCoord1);
				builder.textureCoord(Vec3f::c100 + float(i));

				index[i] = builder.addVertex();
				CHECK_EQUAL(i, index[i]);
			}

			builder.addTriangle(index[0], index[1], index[2]);
		}

		// Get back the buffer pointers, and their element counts
		{	size_t count = 0;
			float* position = reinterpret_cast<float*>(builder.acquireBufferPointer(Mesh::Position, &count));
			CHECK_EQUAL(3u, count);
			CHECK_EQUAL(1.23f, *position);
			builder.releaseBufferPointer(position);
		}

		{	size_t count = 0;
			uint16_t* index = reinterpret_cast<uint16_t*>(builder.acquireBufferPointer(Mesh::Index, &count));
			CHECK_EQUAL(3u, count);
			CHECK(index != nullptr);
			if(index) for(size_t i=0; i<3; ++i)
				CHECK_EQUAL(i, index[i]);
			builder.releaseBufferPointer(index);
		}

		BasicGlWindow window(L"show=0, width=1, height=1");
		MeshPtr mesh = new Mesh(L"");
		builder.commit(*mesh, MeshBuilder::Static);

		CHECK(mesh->format() == builder.format());
		CHECK_EQUAL(3u, mesh->vertexCount());
		CHECK_EQUAL(3u, mesh->indexCount());
	}
}

#include "../../MCD/Render/ChamferBox.h"
TEST(ChamferBox_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=ChamferBox_MeshBuilderTest;width=800;height=600"), mAngle(0)
		{
			mMesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
			chamferBoxBuilder.commit(*mMesh, MeshBuilder::Static);
		}

		sal_override void update(float deltaTime)
		{
			glTranslatef(0.0f, 0.0f, -5.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			mMesh->draw();
		}

		MCD::MeshPtr mMesh;
		float mAngle;
	};

	TestWindow window;
	window.mainLoop();

	CHECK(true);
}

TEST(Quad_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=Quad_MeshBuilderTest;width=800;height=600"), mAngle(0)
		{
			mMesh = new Mesh(L"");
			MeshBuilder builder;

			builder.enable(Mesh::Position | Mesh::Color | Mesh::Normal | Mesh::TextureCoord0 | Mesh::Index);
				builder.textureUnit(Mesh::TextureCoord0);
				builder.textureCoordSize(2);

				builder.reserveVertex(4);
				builder.reserveTriangle(2);

				builder.normal(Vec3f(0, 0, 1));
				builder.position(Vec3f(-1, 1, 1));
				builder.color(ColorRGB8::red);
				builder.textureCoord(Vec2f(0, 1));
				uint16_t idx1 = builder.addVertex();

				builder.position(Vec3f(-1, -1, 1));
				builder.color(ColorRGB8::green);
				builder.textureCoord(Vec2f(0, 0));
				uint16_t idx2 = builder.addVertex();

				builder.position(Vec3f(1, -1, 1));
				builder.color(ColorRGB8::blue);
				builder.textureCoord(Vec2f(1, 0));
				uint16_t idx3 = builder.addVertex();

				builder.position(Vec3f(1, 1, 1));
				builder.color(ColorRGB8::yellow);
				builder.textureCoord(Vec2f(1, 1));
				uint16_t idx4 = builder.addVertex();

				builder.addQuad(idx1, idx2, idx3, idx4);
			builder.commit(*mMesh, MeshBuilder::Static);
		}

		sal_override void update(float deltaTime)
		{
			glTranslatef(0.0f, 0.0f, -5.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			mMesh->draw();
		}

		MCD::MeshPtr mMesh;
		float mAngle;
	};

	TestWindow window;

	// Enable color material
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	window.mainLoop();

	glDisable(GL_COLOR_MATERIAL);

	CHECK(true);
}

// Multiple mesh object are sharing the same vertex buffer
TEST(SharedVertex_MeshBuilderTest)
{
	BasicGlWindow window(L"show=0, width=1, height=1");
	MeshPtr mesh1 = new Mesh(L"");
	MeshPtr mesh2 = new Mesh(L"");
	MeshBuilder vertexBuilder, indexBuilder;

	// Build the vertex
	vertexBuilder.enable(Mesh::Position);
	vertexBuilder.position(Vec3f(-1, 1, 1));
	vertexBuilder.addVertex();
	vertexBuilder.position(Vec3f(-1, -1, 1));
	vertexBuilder.addVertex();
	vertexBuilder.position(Vec3f(1, -1, 1));
	vertexBuilder.addVertex();
	vertexBuilder.position(Vec3f(1, 1, 1));
	vertexBuilder.addVertex();
	vertexBuilder.commit(*mesh1, MeshBuilder::Static);

	// Set mesh2 to share the same vertex buffer as mesh1
	mesh2->setHandlePtr(Mesh::Position, mesh1->handlePtr(Mesh::Position));

	// Index for mesh1
	indexBuilder.enable(Mesh::Index);
	indexBuilder.addTriangle(0, 1, 2);
	indexBuilder.commit(*mesh1, Mesh::Index, MeshBuilder::Static);

	// Index for mesh2
	indexBuilder.clear();
	indexBuilder.enable(Mesh::Index);
	indexBuilder.addTriangle(2, 3, 0);
	indexBuilder.commit(*mesh2, Mesh::Index, MeshBuilder::Static);

	CHECK(true);
}
*/