#include "Pch.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Core/Math/Vec3.h"

using namespace MCD;

TEST(Basic_MeshLoaderTest)
{
	{	// Simple create and destroy a mesh
		MCD::ResourcePtr mesh = new Mesh(L"");
		mesh = nullptr;
	}

	{	// Simple create and destroy a mesh builder
		MeshBuilder builder;
	}

	{	// Simple begin end with nothing
		MeshBuilder builder;
		builder.begin(Mesh::Position | Mesh::Normal | Mesh::Index);
		builder.end();
	}
}

#include "BasicGlWindow.h"
#include "ChamferBox.h"

TEST(Sphere_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			mMesh = new Mesh(L"");
		}

		sal_override void update(float deltaTime)
		{
			glScalef(0.5, 0.5, 0.5);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			Mesh& mesh = static_cast<Mesh&>(*mMesh);
			mesh.draw();
		}

		MCD::ResourcePtr mMesh;
		float mAngle;
	};

	{	// Add some vertex with normal and index
		MeshBuilder builder;
		builder.begin(Mesh::Position | Mesh::Normal | Mesh::Index);
			builder.reserveVertex(4);
			builder.reserveTriangle(2);

			builder.normal(Vec3f(0, 0, 1));
			builder.position(Vec3f(-1, 1, 1));
			uint16_t idx1 = builder.addVertex();

			builder.position(Vec3f(1, 1, 1));
			uint16_t idx2 = builder.addVertex();

			builder.position(Vec3f(1, -1, 1));
			uint16_t idx3 = builder.addVertex();

			builder.position(Vec3f(-1, -1, 1));
			uint16_t idx4 = builder.addVertex();

			builder.addTriangle(idx1, idx2, idx3);
			builder.addTriangle(idx3, idx4, idx1);
		builder.end();

		TestWindow window;
		glPolygonMode(GL_FRONT, GL_LINE);
		ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);

		builder.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);
		chamferBoxBuilder.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);
		glEnable(GL_CULL_FACE);

		// Set up and enable light 0
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

		GLfloat lightPos[] = { -50.f, 0.0f, -50.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		window.mainLoop();
	}
}
