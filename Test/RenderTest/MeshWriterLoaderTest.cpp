#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/MeshWriter.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include <fstream>

using namespace MCD;

TEST(MeshWriterLoaderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow("title=ModelLoaderTest;width=800;height=600;fullscreen=0;FSAA=4")
		{
		}
	};	// TestWindow

	TestWindow window;

	std::auto_ptr<IFileSystem> fs(new RawFileSystem("./Media/"));
	DefaultResourceManager manager(*fs);

	// Setup the chamfer box mesh
	MeshPtr mesh = new Mesh("");
	ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
	MCD_VERIFY(commitMesh(chamferBoxBuilder, *mesh, Mesh::Static));

	// Write it to a tmp file
	{	std::auto_ptr<std::ostream> os(fs->openWrite("tmp.msh"));
		CHECK(MeshWriter::write(*os, *mesh));

		// Consume the reload event casue by the write operation
		manager.processLoadingEvents();
	}

	// Load it back
	MeshPtr mesh2 = dynamic_cast<Mesh*>(manager.load("tmp.msh", IResourceManager::Block).get());
	CHECK(mesh2);

	manager.processLoadingEvents();

	fs.release();

	// Check the result
	CHECK_EQUAL(mesh->bufferCount, mesh2->bufferCount);
	CHECK_EQUAL(mesh->attributeCount, mesh2->attributeCount);
	CHECK_EQUAL(mesh->vertexCount, mesh2->vertexCount);
	CHECK_EQUAL(mesh->indexCount, mesh2->indexCount);

	CHECK_EQUAL(mesh->indexAttrIdx, mesh2->indexAttrIdx);
	CHECK_EQUAL(mesh->positionAttrIdx, mesh2->positionAttrIdx);
	CHECK_EQUAL(mesh->normalAttrIdx, mesh2->normalAttrIdx);
	CHECK_EQUAL(mesh->uv0AttrIdx, mesh2->uv0AttrIdx);

	// TODO: Check the content of the buffers
}
