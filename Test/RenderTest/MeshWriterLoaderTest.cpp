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

	// TODO: Check the content of the buffers
}
