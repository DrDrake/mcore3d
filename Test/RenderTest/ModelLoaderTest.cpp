#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/ResourceLoader.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include <fstream>

using namespace MCD;

TEST(ModelLoaderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow("title=ModelLoaderTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mAngle(0)
		{
			std::auto_ptr<IFileSystem> fs(new RawFileSystem("./Media/"));
			mResourceManager.reset(new DefaultResourceManager(*fs));
			fs.release();
		}

		void loadModel(const char* fileId)
		{
			mModel = dynamic_cast<Model*>(mResourceManager->load(fileId, IResourceManager::NonBlock, 0).get());
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager->processLoadingEvents();

			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			const float scale = 0.2f;
			glScalef(scale, scale, scale);

			if(!mModel)
				return;

			mModel->draw();
		}

		ModelPtr mModel;
		float mAngle;

		std::auto_ptr<DefaultResourceManager> mResourceManager;
	};	// TestWindow

	{
		TestWindow window;

//		window.loadModel("testbox.mesh");
		window.loadModel("Scene/City/scene.3ds");

		window.mainLoop();
	}

	CHECK(true);
}
