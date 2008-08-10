#include "Pch.h"
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

		// TODO: Implement
//		builder.color();
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

TEST(Cube_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			mMesh = new Mesh(L"");
			MeshBuilder builder;

			builder.enable(Mesh::Position | Mesh::Normal | Mesh::TextureCoord0 | Mesh::Index);
				builder.textureUnit(Mesh::TextureCoord0);
				builder.textureCoordSize(2);

				builder.reserveVertex(4);
				builder.reserveTriangle(2);
				builder.textureUnit(Mesh::TextureCoord0);

				builder.normal(Vec3f(0, 0, 1));
				builder.position(Vec3f(-1, 1, 1));
				builder.textureCoord(Vec2f(0, 1));
				uint16_t idx1 = builder.addVertex();

				builder.position(Vec3f(-1, -1, 1));
				builder.textureCoord(Vec2f(0, 0));
				uint16_t idx2 = builder.addVertex();

				builder.position(Vec3f(1, -1, 1));
				builder.textureCoord(Vec2f(1, 0));
				uint16_t idx3 = builder.addVertex();

				builder.position(Vec3f(1, 1, 1));
				builder.textureCoord(Vec2f(1, 1));
				uint16_t idx4 = builder.addVertex();

				builder.addTriangle(idx1, idx2, idx3);
				builder.addTriangle(idx3, idx4, idx1);
			builder.commit(*mMesh, MeshBuilder::Static);
		}

		sal_override void update(float deltaTime)
		{
			glTranslatef(0.0f, 0.0f, -5.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			glColor3f(1, 0, 0);
			mMesh->draw();
		}

		MCD::MeshPtr mMesh;
		float mAngle;
	};

	TestWindow window;
	window.mainLoop();
}

#include "../../MCD/Render/Max3dsLoader.h"
#include "../../MCD/Render/JpegLoader.h"
#include "../../MCD/Render/PngLoader.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include <fstream>

TEST(Sphere_MeshBuilderTest)
{
	class JpegFactory : public ResourceManager::IFactory
	{
	public:
		sal_override ResourcePtr createResource(const Path& path) {
			return new Texture(path);
		}

		sal_override IResourceLoader* createLoader() {
			return new JpegLoader;
		}
	};	// JpegFactory

	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			std::auto_ptr<IFileSystem> fs(new RawFileSystem(L"./"));
			mResourceManager.reset(new ResourceManager(*fs));
			fs.release();
			mResourceManager->associateFactory(L"JPG", new JpegFactory);
		}

		bool load3ds(const char* fileName)
		{
			std::ifstream is(fileName, std::ios_base::binary);
			if(!is.is_open())
				return false;

			Max3dsLoader loader(is, mResourceManager.get());
			loader.commit(mModel, MeshBuilder::Static);

			return true;
		}

		void processResourceLoadingEvents()
		{
			while(true) {ResourceManager::Event e = mResourceManager->popEvent();
			if(e.loader) {
				// Allow one resource to commit for each frame
				e.loader->commit(*e.resource);
			} else
				break;
			}
		}

		sal_override void update(float deltaTime)
		{
			processResourceLoadingEvents();

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, -50.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			const float scale = 1.0f;
			glScalef(scale, scale, scale);

			mModel.draw();
			glPopMatrix();
		}

		Model mModel;
		float mAngle;

		std::auto_ptr<ResourceManager> mResourceManager;
	};

	{
		TestWindow window;

		window.load3ds("titanic.3DS");
//		window.load3ds("titanic2.3DS");
//		window.load3ds("spaceship.3DS");
//		window.load3ds("N.3ds");
//		window.load3ds("ship^kiy.3ds");
//		window.load3ds("Alfa Romeo.3ds");
//		window.load3ds("Nissan350Z.3ds");
//		window.load3ds("Nathalie aguilera Boing 747.3DS");
//		window.load3ds("Dog 1 N280708.3ds");
//		window.load3ds("Leon N300708.3DS");
//		window.load3ds("Ford N120208.3ds");
//		window.load3ds("musai.3DS");

		// Set up and enable light 0
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		GLfloat lightPos[] = { -50.f, 0.0f, 40.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		window.mainLoop();
	}
}
