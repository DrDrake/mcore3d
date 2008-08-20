#include "Pch.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/MeshBuilder.h"

using namespace MCD;

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
			for(size_t i=0; i<3; ++i)
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

#include "ChamferBox.h"
TEST(ChamferBox_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(), mAngle(0)
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
			BasicGlWindow(), mAngle(0)
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
}
